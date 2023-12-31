#include "grid_client_server_remote_connector.hpp"

namespace xios
{
  /** 
   * \brief class constructor. 
   * \param srcView List of sources views.
   * \param dstView List of remotes views.
   * \param localComm Local MPI communicator
   * \param remoteSize Size of the remote communicator
   */ 
  CGridClientServerRemoteConnector::CGridClientServerRemoteConnector( vector<shared_ptr<CLocalView> >& srcFullView, vector<shared_ptr<CLocalView>>& srcWorkflowView, 
                                                                      vector<shared_ptr<CDistributedView>>& dstView, MPI_Comm localComm, int remoteSize) 
                       : CGridRemoteConnector(srcFullView, dstView, localComm, remoteSize) , srcWorkflowView_(srcWorkflowView)
  {}


  void CGridClientServerRemoteConnector::computeConnector(bool eliminateRedondant)
  {
    if (eliminateRedondant)
    {
      auto workflowRemoteConnector=make_shared<CGridRemoteConnector>(srcWorkflowView_,dstView_,localComm_,remoteSize_) ;
      workflowRemoteConnector->computeConnector() ;
      computeViewDistribution() ;
    
      for(int i=0;i<srcView_.size();i++) isSrcViewDistributed_[i] =  isSrcViewDistributed_[i] || workflowRemoteConnector->getIsSrcViewDistributed()[i]  ;
      computeConnectorMethods() ;
      computeRedondantRanks() ;

      for(auto& rank : rankToRemove_)
        if (workflowRemoteConnector->getRankToRemove().count(rank)!=0)
          for(auto& element : elements_) element.erase(rank) ;
    }
    else 
    {
      computeViewDistribution() ;
      computeConnectorRedundant() ;
    }
  }
 
  void CGridClientServerRemoteConnector::computeConnectorOut()
  {
    set<int> workflowRankToRemove ;
    vector<bool> workflowIsSrcViewDistributed ;
    {
      auto workflowRemoteConnector=make_shared<CGridRemoteConnector>(srcWorkflowView_,dstView_,localComm_,remoteSize_) ;
      workflowRemoteConnector->computeViewDistribution() ;
      workflowRemoteConnector->computeConnectorMethods(false) ;
      workflowRemoteConnector->computeRedondantRanks(false) ;
      workflowRankToRemove = workflowRemoteConnector->getRankToRemove() ;
      workflowIsSrcViewDistributed = workflowRemoteConnector->getIsSrcViewDistributed() ;
    }
    
    computeViewDistribution() ;
    
    for(int i=0;i<srcView_.size();i++) isSrcViewDistributed_[i] =  isSrcViewDistributed_[i] || workflowIsSrcViewDistributed[i]  ;
    computeConnectorMethods(false) ;
    computeRedondantRanks(false) ;

    for(auto& rank : rankToRemove_)
      if (workflowRankToRemove.count(rank)!=0)
        for(auto& element : elements_) element.erase(rank) ;
  }

  void CGridClientServerRemoteConnector::computeConnectorIn()
  {
    set<int> workflowRankToRemove ;
    vector<bool> workflowIsSrcViewDistributed ;
    {
      auto workflowRemoteConnector=make_shared<CGridRemoteConnector>(srcWorkflowView_,dstView_,localComm_,remoteSize_) ;
      workflowRemoteConnector->computeViewDistribution() ;
      workflowRemoteConnector->computeConnectorMethods(true) ;
      workflowRemoteConnector->computeRedondantRanks(true) ;
      workflowRankToRemove = workflowRemoteConnector->getRankToRemove() ;
      workflowIsSrcViewDistributed = workflowRemoteConnector->getIsSrcViewDistributed() ;
    }

    computeViewDistribution() ;
    
    for(int i=0;i<srcView_.size();i++) isSrcViewDistributed_[i] =  isSrcViewDistributed_[i] || workflowIsSrcViewDistributed[i]  ;
    computeConnectorMethods(true) ;
    computeRedondantRanks(true) ;

    for(auto& rank : rankToRemove_)
      if (workflowRankToRemove.count(rank)!=0)
        for(auto& element : elements_) element.erase(rank) ;
  }
}