#ifndef __CONTEXT_SERVER_HPP__
#define __CONTEXT_SERVER_HPP__
#include "xios_spl.hpp"
#include "event_server.hpp"
#include "buffer_server.hpp"
#include "mpi.hpp"

namespace xios
{
  class CContext ;

  class CContextServer
  {
    public:

    CContextServer(CContext* parent, MPI_Comm intraComm, MPI_Comm interComm) ;

    bool eventLoop(bool enableEventsProcessing = true);
    void listen(void) ;
    void checkPendingRequest(void) ;
    void processEvents(void) ;
    bool hasFinished(void);
    void dispatchEvent(CEventServer& event) ;
    void setPendingEvent(void) ;
    bool hasPendingEvent(void) ;

    void processRequest(int rank, char* buff,int count) ;

    MPI_Comm intraComm ;
    int intraCommSize ;
    int intraCommRank ;

    MPI_Comm interComm ;
    int commSize ;

    map<int,CServerBuffer*> buffers ;
    map<int,MPI_Request> pendingRequest ;
    map<int,char*> bufferRequest ;

    map<size_t,CEventServer*> events ;
    size_t currentTimeLine ;
    CContext* context ;
      
    bool finished ;
    bool pendingEvent ;
    bool scheduled  ;    /*!< event of current timeline is alreading scheduled ? */
    size_t hashId ;

    static size_t getTotalBuf(void);

    ~CContextServer() ;

    private:
    std::map<int, StdSize> mapBufferSize_;
    static size_t totalBuf_ ;  /*!< Total memory allocated by servers per context.*/

  } ;

}

#endif
