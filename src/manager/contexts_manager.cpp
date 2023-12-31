#include "contexts_manager.hpp"
#include "cxios.hpp"
#include "ressources_manager.hpp"
#include "pool_ressource.hpp"
#include "services.hpp"
#include "server_context.hpp"
#include "servers_ressource.hpp"
#include "server.hpp"
#include "timer.hpp"
#include <functional>


namespace xios
{
  using namespace std ;

  CContextsManager::CContextsManager(bool isXiosServer)
  {
    xiosComm_ = CXios::getXiosComm()  ;
    
    int commRank ;  
    MPI_Comm_rank(xiosComm_, &commRank) ;
    if (commRank==0 && isXiosServer) MPI_Comm_rank(xiosComm_, &commRank) ; 
    else commRank=0 ;
    MPI_Allreduce(&commRank, &managerGlobalLeader_, 1, MPI_INT, MPI_SUM, xiosComm_) ;

    MPI_Comm_rank(xiosComm_, &commRank) ;
    winNotify_ = new CWindowManager(xiosComm_, maxBufferSize_,"CContextsManager::winNotify_") ;
    winNotify_->updateToExclusiveWindow(commRank, this, &CContextsManager::notificationsDumpOut) ;
   

    winContexts_ = new CWindowManager(xiosComm_, maxBufferSize_,"CContextsManager::winContexts_") ;
    winContexts_->updateToExclusiveWindow(commRank, this, &CContextsManager::contextsDumpOut) ;
  
    MPI_Barrier(xiosComm_)  ;    
  }


  CContextsManager::~CContextsManager()
  {
    delete winNotify_ ;
    delete winContexts_ ;
  }

  bool CContextsManager::createServerContext(const std::string& poolId, const std::string& serviceId, const int& partitionId,
                                             const string& contextId, bool wait)
  {
    int serviceLeader ;
    auto servicesManager = CXios::getServicesManager() ;
   
    bool ok=servicesManager->getServiceLeader(poolId, serviceId, partitionId, serviceLeader) ;

    info(40)<<"CContextsManager::createServerContext : waiting for service leader ;  serviceId : "<<serviceId<<endl ;
    if (wait)
    {
      while (!ok) 
      {
        CXios::getDaemonsManager()->servicesEventLoop() ;
        ok=servicesManager->getServiceLeader(poolId, serviceId, partitionId, serviceLeader) ;
      }
    }

    if (ok) 
    {
      notifyType_=NOTIFY_CREATE_CONTEXT ;
      notifyCreateContext_=make_tuple(poolId, serviceId, partitionId, contextId) ;
      info(40)<<"CContextsManager::createServerContext : notification create_context to service leader "<<serviceLeader<<", serviceId : "<<serviceId<<", contextId "<<contextId<<endl ;
      sendNotification(serviceLeader) ;
      return true ;
    }
    else return false ;
  }


  bool CContextsManager::createServerContextIntercomm(const string& poolId, const string& serviceId, const int& partitionId,
                                                      const string& contextId, const string& sourceContext, bool wait)
  {
    int contextLeader ;
    bool ok ;
    int remoteLeader ;
    MPI_Comm_rank(xiosComm_, &remoteLeader) ;
    
    int type ;
    info(40)<<"CContextsManager::createServerContextIntercomm : waiting for context leader ;  contextId : "<<contextId<<endl ;
    ok=CXios::getServicesManager()->getServiceType(poolId,serviceId, 0, type) ;
    if (ok) ok=getContextLeader(getServerContextName(poolId, serviceId, partitionId, type, contextId), contextLeader) ;
    if (wait)
    {
      while (!ok) 
      {
        CXios::getDaemonsManager()->servicesEventLoop() ;
        ok=CXios::getServicesManager()->getServiceType(poolId,serviceId, 0, type) ;
        if (ok) ok=getContextLeader(getServerContextName(poolId, serviceId, partitionId, type, contextId), contextLeader) ;
      }
    }
    
    if (ok) 
    {
      notifyType_=NOTIFY_CREATE_INTERCOMM ;
      notifyCreateIntercomm_=make_tuple(poolId, serviceId, partitionId, contextId, remoteLeader, sourceContext) ;
      info(40)<<"CContextsManager::createServerContextIntercomm : notification create_intercomm to context leader : "<<contextLeader<<", contextId :"<<contextId<<endl ;
      sendNotification(contextLeader) ;
      return true ;
    }
    else return false ;
  }

  void CContextsManager::sendNotification(int rank)
  {
    winNotify_->lockWindowExclusive(rank) ;
    winNotify_->pushToLockedWindow(rank, this, &CContextsManager::notificationsDumpOut) ;
    winNotify_->unlockWindowExclusive(rank) ;
  }

  
  void CContextsManager::notificationsDumpOut(CBufferOut& buffer)
  {
    
    buffer.realloc(maxBufferSize_) ;
    
    if (notifyType_==NOTIFY_CREATE_CONTEXT)
    {
      auto& arg=notifyCreateContext_ ;
      buffer << notifyType_<< get<0>(arg) << get<1>(arg) << std::get<2>(arg) << get<3>(arg) ;
    }
    else if (notifyType_==NOTIFY_CREATE_INTERCOMM)
    {
      auto& arg=notifyCreateIntercomm_ ;
      buffer << notifyType_<< get<0>(arg) << get<1>(arg) << std::get<2>(arg) << get<3>(arg) << get<4>(arg)<< get<5>(arg) ;
    }
  }

  void CContextsManager::notificationsDumpIn(CBufferIn& buffer)
  {
    if (buffer.bufferSize() == 0) notifyType_= NOTIFY_NOTHING ;
    else
    {
      buffer>>notifyType_;
      if (notifyType_==NOTIFY_CREATE_CONTEXT)
      {
        auto& arg=notifyCreateContext_ ;
        buffer >> get<0>(arg) >> get<1>(arg) >> std::get<2>(arg)>> get<3>(arg) ;
      }
      else if (notifyType_==NOTIFY_CREATE_INTERCOMM)
      {
        auto& arg=notifyCreateIntercomm_ ;
        buffer >> get<0>(arg) >> get<1>(arg) >> std::get<2>(arg) >> get<3>(arg) >> get<4>(arg) >> get<5>(arg);
      }
    }

  }

  void CContextsManager::eventLoop(void)
  {
    CTimer::get("CContextsManager::eventLoop").resume();
    int flag ;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    double time=MPI_Wtime() ;
    if (time-lastEventLoop_ > eventLoopLatency_) 
    {
      checkNotifications() ;
      lastEventLoop_=time ;
    }
    CTimer::get("CContextsManager::eventLoop").suspend();
  }
  
  void CContextsManager::checkNotifications(void)
  {
    int commRank ;
    MPI_Comm_rank(xiosComm_, &commRank) ;
    winNotify_->popFromExclusiveWindow(commRank, this, &CContextsManager::notificationsDumpIn) ;
    if (notifyType_==NOTIFY_CREATE_CONTEXT) createServerContext() ;
    else if (notifyType_==NOTIFY_CREATE_INTERCOMM) createServerContextIntercomm() ;

  }

  void CContextsManager::createServerContext(void)
  {
    info(40)<<"CContextsManager::createServerContext : receive create server context notification"<<endl ;
    auto arg=notifyCreateContext_ ;
    CXios::getPoolRessource()->getService(get<1>(arg), get<2>(arg))
                             ->createContext(get<0>(arg), get<1>(arg), get<2>(arg), get<3>(arg)) ;
  
  }

  void CContextsManager::createServerContextIntercomm(void)
  {
    info(40)<<"CContextsManager::createServerContext : receive create intercomm context notification"<<endl ;
    auto arg=notifyCreateIntercomm_ ;
    CXios::getPoolRessource()->getService(get<1>(arg), get<2>(arg))
                             ->getServerContext(get<3>(arg))
                             ->createIntercomm(get<4>(arg), get<5>(arg)) ;
  }              

  string CContextsManager::getServerContextName(const string& poolId, const string& serviceId, const int& partitionId, 
                                                const int& type, const string& contextId)
  {
    if (type==CServicesManager::CLIENT) return contextId;
    else
    {
      ostringstream oss;
      oss<<partitionId;
      return poolId+"__"+serviceId+"_"+oss.str()+"__"+contextId;
    }
  }

  void CContextsManager::registerContext(const string& fullContextId, const SRegisterContextInfo& contextInfo)
  {
    winContexts_->lockWindowExclusive(managerGlobalLeader_) ;
    winContexts_->updateFromLockedWindow(managerGlobalLeader_, this, &CContextsManager::contextsDumpIn) ;
    winContexts_->flushWindow(managerGlobalLeader_) ;
    contexts_[fullContextId] = contextInfo ;
    winContexts_->updateToLockedWindow(managerGlobalLeader_, this, &CContextsManager::contextsDumpOut) ;
    winContexts_->unlockWindowExclusive(managerGlobalLeader_) ;
  }

  bool CContextsManager::getContextInfo(const string& fullContextId, SRegisterContextInfo& contextInfo, MPI_Comm comm)
  {
    bool ret ;
    int commRank=0 ;
    if (comm!=MPI_COMM_NULL) MPI_Comm_rank(comm, &commRank) ;

    if (commRank==0)
    {

      winContexts_->updateFromSharedWindow(managerGlobalLeader_, this, &CContextsManager::contextsDumpIn) ;

      auto it=contexts_.find(fullContextId) ;
      if ( it == contexts_.end()) ret=false ;
      else
      {
        contextInfo=it->second ; 
        ret=true ;
      }
    }
    
    if (comm!=MPI_COMM_NULL) 
    {
      MPI_Bcast(&ret,1,MPI_INT,0,comm) ;
      if (ret)
      {
        MPI_Bcast(&contextInfo.leader,1,MPI_INT,0,comm) ;
        MPI_Bcast(&contextInfo.size,1,MPI_INT,0,comm) ;
        MPI_Bcast_string(contextInfo.poolId,0,comm) ;
        MPI_Bcast_string(contextInfo.serviceId,0,comm) ;
        MPI_Bcast(&contextInfo.serviceType,1,MPI_INT,0,comm) ;
        MPI_Bcast(&contextInfo.partitionId,1,MPI_INT,0,comm) ;
        MPI_Bcast_string(contextInfo.id,0,comm) ;
      }
    }
    return ret ;
  }

  bool CContextsManager::getContextLeader(const string& fullContextId, int& leader, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) leader=contextInfo.leader ;
    return ret ;
  }

  bool CContextsManager::getContextSize(const string& fullContextId, int& size, MPI_Comm comm)
  {

    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) size=contextInfo.size ;
    return ret ;
  }

  bool CContextsManager::getContextPoolId(const string& fullContextId, string& poolId, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) poolId=contextInfo.poolId ;
    return ret ;
  }

  bool CContextsManager::getContextServiceId(const string& fullContextId, string& serviceId, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) serviceId=contextInfo.serviceId ;
    return ret ;
  }

  bool CContextsManager::getContextPartitionId(const string& fullContextId, int& partitionId, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) partitionId=contextInfo.partitionId ;
    return ret ;
  }
  
  bool CContextsManager::getContextServiceType(const string& fullContextId, int& serviceType, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) serviceType=contextInfo.serviceType ;
    return ret ;
  }

  bool CContextsManager::getContextId(const string& fullContextId, string& contextId, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    bool ret=getContextInfo(fullContextId, contextInfo) ;
    if (ret) contextId=contextInfo.id ;
    return ret ;
  }


  bool CContextsManager::hasContext(const string& fullContextId, MPI_Comm comm)
  {
    SRegisterContextInfo contextInfo ;
    return getContextInfo(fullContextId, contextInfo) ;
  }

  void CContextsManager::contextsDumpOut(CBufferOut& buffer)
  {
    buffer.realloc(maxBufferSize_) ;
    buffer<<(int)contexts_.size();
    
    for(auto it=contexts_.begin();it!=contexts_.end(); ++it)
    { 
      auto key = it->first ;
      auto val = it->second ; 
      buffer << key << val.poolId<<val.serviceId<<val.partitionId<<val.serviceType<<val.id<<val.size<<val.leader  ;
    }
  } 

  void CContextsManager::contextsDumpIn(CBufferIn& buffer)
  {
    std::string contextId ;
    SRegisterContextInfo ci;
    int size; 
    int leader ;

    contexts_.clear() ;
    int nbContexts ;
    buffer>>nbContexts ;
    for(int i=0;i<nbContexts;i++) 
    {
      buffer>>contextId>>ci.poolId>>ci.serviceId>>ci.partitionId>>ci.serviceType>>ci.id>>ci.size>>ci.leader ;
      contexts_[contextId]=ci ;
    }

  }
}
