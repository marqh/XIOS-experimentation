#ifndef __CONTEXT_CLIENT_HPP__
#define __CONTEXT_CLIENT_HPP__

#include "xios_spl.hpp"
#include "buffer_out.hpp"
#include "buffer_in.hpp"
#include "buffer_client.hpp"
#include "event_client.hpp"
#include "event_server.hpp"
#include "mpi.hpp"
#include "registry.hpp"

namespace xios
{
  class CContext;
  class CContextServer ;
  /*!
  \class CContextClient
  A context can be both on client and on server side. In order to differenciate the role of
  context on each side, e.x client sending events, server receiving and processing events, there is a need of
  concrete "context" classes for both sides.
  CContextClient processes and sends events from client to server where CContextServer receives these events
  and processes them.
  */
  class CContextClient
  {
    public:
      // Contructor
      CContextClient(CContext* parent, MPI_Comm intraComm, MPI_Comm interComm, CContext* parentServer = 0);

      // Send event to server
      void sendEvent(CEventClient& event);
      void waitEvent(list<int>& ranks);
      void waitEvent_old(list<int>& ranks);

      // Functions to set/get buffers
      bool getBuffers(const size_t timeLine, const list<int>& serverList, const list<int>& sizeList, list<CBufferOut*>& retBuffers, bool nonBlocking = false);
      void newBuffer(int rank);
      bool checkBuffers(list<int>& ranks);
      bool checkBuffers(void);
      void eventLoop(void) ;
      void callGlobalEventLoop() ;
      void releaseBuffers(void);
      bool havePendingRequests(void);
      bool havePendingRequests(list<int>& ranks) ;

      bool isServerLeader(void) const;
      bool isServerNotLeader(void) const;
      const std::list<int>& getRanksServerLeader(void) const;
      const std::list<int>& getRanksServerNotLeader(void) const;

  /*!
   * Check if the attached mode is used.
   *
   * \return true if and only if attached mode is used
   */
      bool isAttachedModeEnabled() const { return isAttached_ ; } 

      static void computeLeader(int clientRank, int clientSize, int serverSize,
                                std::list<int>& rankRecvLeader,
                                std::list<int>& rankRecvNotLeader);

      // Close and finalize context client
//      void closeContext(void);  Never been implemented.
      bool isNotifiedFinalized(void) ;
      void finalize(void);

      void setBufferSize(const std::map<int,StdSize>& mapSize);

      int getRemoteSize(void) {return serverSize;}
      int getServerSize(void) {return serverSize;}
      MPI_Comm getIntraComm(void)  {return intraComm ;} 
      int getIntraCommSize(void) {return clientSize ;}
      int getIntraCommRank(void) {return clientRank ;}

      /*! set the associated server (dual chanel client/server) */      
      void setAssociatedServer(CContextServer* associatedServer) { associatedServer=associatedServer_;}
      /*! get the associated server (dual chanel client/server) */      
      CContextServer* getAssociatedServer(void) { return associatedServer_;}
      void setGrowableBuffer(void) { isGrowableBuffer_=true;}
      void setFixedBuffer(void) { isGrowableBuffer_=false;}
    public:
      CContext* context_; //!< Context for client

      size_t timeLine; //!< Timeline of each event

      int clientRank; //!< Rank of current client

      int clientSize; //!< Size of client group

      int serverSize; //!< Size of server group

      MPI_Comm interComm; //!< Communicator of server group (interCommunicator)

      MPI_Comm interCommMerged_; //!< Communicator of the client group + server group (intraCommunicator) needed for one sided communication.
      MPI_Comm commSelf_ ; //!< Communicator for proc alone from interCommMerged 

      MPI_Comm intraComm; //!< Communicator of client group

      map<int,CClientBuffer*> buffers; //!< Buffers for connection to servers

      bool pureOneSided ; //!< if true, client will communicated with servers only trough one sided communication. Otherwise the hybrid mode P2P /One sided is used.

      size_t hashId_ ; //!< hash id on the context client that will be used for context server to identify the remote calling context client.

    private:
      void lockBuffers(list<int>& ranks) ;
      void unlockBuffers(list<int>& ranks) ;
      
      //! Mapping of server and buffer size for each connection to server
      std::map<int,StdSize> mapBufferSize_;
      //! Maximum event sizes estimated for each connection to server
      std::map<int,StdSize> maxEventSizes;
      //! Maximum number of events that can be buffered
      StdSize maxBufferedEvents;

      //! Context for server (Only used in attached mode)
      CContext* parentServer;

      //! List of server ranks for which the client is leader
      std::list<int> ranksServerLeader;

      //! List of server ranks for which the client is not leader
      std::list<int> ranksServerNotLeader;

      std::map<int, MPI_Comm> winComm_ ; //! Window communicators
      std::map<int, std::vector<MPI_Win> >windows_ ; //! one sided mpi windows to expose client buffers to servers == windows[nbServers][2]
      bool isAttached_ ;
      CContextServer* associatedServer_ ; //!< The server associated to the pair client/server
      bool isGrowableBuffer_ = true ;

      double latency_=0e-2 ;

      bool locked_ = false ; //!< The context client is locked to avoid recursive checkBuffer
  };
}

#endif // __CONTEXT_CLIENT_HPP__
