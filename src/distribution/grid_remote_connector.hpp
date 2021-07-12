#ifndef __GRID_REMOTE_CONNECTOR_HPP__
#define __GRID_REMOTE_CONNECTOR_HPP__

#include "xios_spl.hpp"
#include "array_new.hpp"
#include "mpi.hpp"
#include "local_view.hpp"
#include "distributed_view.hpp"
#include "context_client.hpp"


namespace xios
{
 
  class CGridRemoteConnector
  {

    public:

      CGridRemoteConnector(vector<CLocalView*>& srcView, vector<CDistributedView*>& dstView, MPI_Comm localComm, int remoteSize) ;
      CGridRemoteConnector(vector<CLocalView*>& srcView, vector<CLocalView*>& dstView, MPI_Comm localComm, int remoteSize) ;
      void computeViewDistribution(void) ;
      void computeConnector(void) ;
      void computeGenericMethod(vector<CLocalView*>& srcView, vector<CDistributedView*>& dstView, vector<int>& indElements) ;
      void computeSrcDstNonDistributed(int i, map<int,bool>& ranks) ;
      void computeDstNonDistributed(int i, map<int,bool>& ranks) ;
      void computeSrcNonDistributed(int i) ;
      void removeRedondantRanks(void) ;
      std::map<int, CArray<size_t,1>>& getDistributedGlobalIndex(int pos) { return elements_[pos] ;} 

    private:
  
    /** 
     * Source views composing the source grid. The vector store an internal copy of pointer elements composing the grid. 
     * It is feed at construction time
     */
      vector<CLocalView*> srcView_ ;

    /** 
     * Destination views composing the source grid. The vector store an internal copy of pointer elements composing the grid
     * It is feed at construction time
     */
      vector<CDistributedView*> dstView_ ;

    /**  
     * The list of global indices to send to each rank of the remote view (servers). The vector store the information for each element, and the map 
     * specify a list of global indices to send to each rank of the remote view.
     * size of element_[] -> number of elements composing the grids (source/destination)
     * element[i][rank] == CArray<size_t,1> -> list of global indices  to send to the remote process of rank \b rank for the view \b i
     * The is computed when calling computeConnector internal methods
     * map can be returned trough the public accessor : getDistributedGlobalIndex(int)
     */
      vector<map<int, CArray<size_t,1>>> elements_ ;
    
      /**
      /* internal copy of the local communicator (client workflow). Feed at construction time.
       */
      MPI_Comm localComm_ ;

      /**
      /* size of the remote communicator (== nb of servers). Feed at consctruction time
       */
      int remoteSize_ ;

      /**
      /* for each view composing the source grid, the vector store the information about the distribution of the element, i.e. 
       * if each ranks of the local view has exactly the same global indices than each other. This is computed when calling  
       * \b computeViewDistribution method.  
       */
      vector<bool> isSrcViewDistributed_ ;

     /**
      /* for each view composing the destination grid, the vector store the information about the distribution of the element, i.e. 
       * if each ranks of the remote view has exactly the same global indices than each other. This is computed when calling  
       * \b computeViewDistribution method. 
       */
      vector<bool> isDstViewDistributed_ ;
      
    
  } ;

}

#endif