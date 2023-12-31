#include "legacy_context_server.hpp"
#include "buffer_in.hpp"
#include "type.hpp"
#include "context.hpp"
#include "object_template.hpp"
#include "group_template.hpp"
#include "attribute_template.hpp"
#include "domain.hpp"
#include "field.hpp"
#include "file.hpp"
#include "grid.hpp"
#include "mpi.hpp"
#include "tracer.hpp"
#include "timer.hpp"
#include "cxios.hpp"
#include "event_scheduler.hpp"
#include "server.hpp"
#include "servers_ressource.hpp"
#include "pool_ressource.hpp"
#include "services.hpp"
#include "contexts_manager.hpp"
#include "timeline_events.hpp"

#include <boost/functional/hash.hpp>
#include <random>
#include <chrono>


namespace xios
{
  using namespace std ;

  CLegacyContextServer::CLegacyContextServer(CContext* parent,MPI_Comm intraComm_,MPI_Comm interComm_) 
    : CContextServer(parent, intraComm_, interComm_),
      isProcessingEvent_(false)
  {
 
    xios::MPI_Comm_dup(intraComm, &processEventBarrier_) ;
    CXios::getMpiGarbageCollector().registerCommunicator(processEventBarrier_) ;

    currentTimeLine=1;
    scheduled=false;
    finished=false;

    xios::MPI_Intercomm_merge(interComm_,true,&interCommMerged_) ;
    CXios::getMpiGarbageCollector().registerCommunicator(interCommMerged_) ;
    xios::MPI_Comm_split(intraComm_, intraCommRank, intraCommRank, &commSelf_) ; // for windows
    CXios::getMpiGarbageCollector().registerCommunicator(commSelf_) ;
  
    itLastTimeLine=lastTimeLine.begin() ;

    pureOneSided=CXios::getin<bool>("pure_one_sided",false); // pure one sided communication (for test)
  }
 
  void CLegacyContextServer::setPendingEvent(void)
  {
    pendingEvent=true;
  }

  bool CLegacyContextServer::hasPendingEvent(void)
  {
    return pendingEvent;
  }

  bool CLegacyContextServer::hasFinished(void)
  {
    return finished;
  }

  bool CLegacyContextServer::eventLoop(bool enableEventsProcessing /*= true*/)
  {
    CTimer::get("listen request").resume();
    listen();
    CTimer::get("listen request").suspend();
    CTimer::get("check pending request").resume();
    checkPendingRequest();
    checkPendingProbe() ;
    CTimer::get("check pending request").suspend();
    CTimer::get("check event process").resume();
    processEvents(enableEventsProcessing);
    CTimer::get("check event process").suspend();
    return finished;
  }

 void CLegacyContextServer::listen(void)
  {
    int rank;
    int flag;
    int count;
    char * addr;
    MPI_Status status;
    MPI_Message message ;
    map<int,CServerBuffer*>::iterator it;
    bool okLoop;

    traceOff();
    MPI_Improbe(MPI_ANY_SOURCE, 20, interCommMerged_,&flag,&message, &status);
    traceOn();
    if (flag==true) listenPendingRequest(message, status) ;
  }

  bool CLegacyContextServer::listenPendingRequest( MPI_Message &message, MPI_Status& status)
  {
    int count;
    char * addr;
    map<int,CServerBuffer*>::iterator it;
    int rank=status.MPI_SOURCE ;

    it=buffers.find(rank);
    if (it==buffers.end()) // Receive the buffer size and allocate the buffer
    {
      MPI_Aint recvBuff[4] ;
      MPI_Mrecv(recvBuff, 4, MPI_AINT,  &message, &status);
      remoteHashId_ = recvBuff[0] ;
      StdSize buffSize = recvBuff[1];
      vector<MPI_Aint> winBufferAddress(2) ;
      winBufferAddress[0]=recvBuff[2] ; winBufferAddress[1]=recvBuff[3] ;
      mapBufferSize_.insert(std::make_pair(rank, buffSize));

      // create windows dynamically for one-sided
      int dummy ;
      MPI_Send(&dummy, 0, MPI_INT, rank, 21,interCommMerged_) ;
      CTimer::get("create Windows").resume() ;
      MPI_Comm interComm ;
      int tag = 0 ;
      xios::MPI_Intercomm_create(commSelf_, 0, interCommMerged_, rank, tag , &interComm) ;
      xios::MPI_Intercomm_merge(interComm, true, &winComm_[rank]) ;
      xios::MPI_Comm_free(&interComm) ;
      windows_[rank].resize(2) ;
      //MPI_Win_create_dynamic(MPI_INFO_NULL, winComm_[rank], &windows_[rank][0]);
      //CXios::getMpiGarbageCollector().registerWindow(windows_[rank][0]) ;
      //MPI_Win_create_dynamic(MPI_INFO_NULL, winComm_[rank], &windows_[rank][1]);
      //CXios::getMpiGarbageCollector().registerWindow(windows_[rank][1]) ;
      windows_[rank][0] = new CWindowDynamic() ;
      windows_[rank][1] = new CWindowDynamic() ;
      windows_[rank][0] -> create(winComm_[rank]) ;
      windows_[rank][1] -> create(winComm_[rank]) ;
      windows_[rank][0] -> setWinBufferAddress(winBufferAddress[0],0) ;
      windows_[rank][1] -> setWinBufferAddress(winBufferAddress[1],0) ;
      CTimer::get("create Windows").suspend() ;
      CXios::getMpiGarbageCollector().registerCommunicator(winComm_[rank]) ;
      MPI_Barrier(winComm_[rank]) ;

      it=(buffers.insert(pair<int,CServerBuffer*>(rank, new CServerBuffer(rank, windows_[rank], winBufferAddress, 0, buffSize)))).first;
      lastTimeLine[rank]=0 ;
      itLastTimeLine=lastTimeLine.begin() ;

      return true;
    }
    else
    {
        std::pair<MPI_Message,MPI_Status> mypair(message,status) ;
        pendingProbe[rank].push_back(mypair) ;
        return false;
    }
  }

  void CLegacyContextServer::checkPendingProbe(void)
  {
    
    list<int> recvProbe ;
    list<int>::iterator itRecv ;
    map<int, list<std::pair<MPI_Message,MPI_Status> > >::iterator itProbe;

    for(itProbe=pendingProbe.begin();itProbe!=pendingProbe.end();itProbe++)
    {
      int rank=itProbe->first ;
      if (pendingRequest.count(rank)==0)
      {
        MPI_Message& message = itProbe->second.front().first ;
        MPI_Status& status = itProbe->second.front().second ;
        int count ;
        MPI_Get_count(&status,MPI_CHAR,&count);
        map<int,CServerBuffer*>::iterator it = buffers.find(rank);
        if ( (it->second->isBufferFree(count) && !it->second->isResizing()) // accept new request if buffer is free
          || (it->second->isResizing() && it->second->isBufferEmpty()) )    // or if resizing wait for buffer is empty
        {
          char * addr;
          addr=(char*)it->second->getBuffer(count);
          MPI_Imrecv(addr,count,MPI_CHAR, &message, &pendingRequest[rank]);
          bufferRequest[rank]=addr;
          recvProbe.push_back(rank) ;
          itProbe->second.pop_front() ;
        }
      }
    }

    for(itRecv=recvProbe.begin(); itRecv!=recvProbe.end(); itRecv++) if (pendingProbe[*itRecv].empty()) pendingProbe.erase(*itRecv) ;
  }


  void CLegacyContextServer::checkPendingRequest(void)
  {
    map<int,MPI_Request>::iterator it;
    list<int> recvRequest;
    list<int>::iterator itRecv;
    int rank;
    int flag;
    int count;
    MPI_Status status;
   
    if (!pendingRequest.empty()) CTimer::get("receiving requests").resume();
    else CTimer::get("receiving requests").suspend();

    for(it=pendingRequest.begin();it!=pendingRequest.end();it++)
    {
      rank=it->first;
      traceOff();
      MPI_Test(& it->second, &flag, &status);
      traceOn();
      if (flag==true)
      {
        buffers[rank]->updateCurrentWindows() ;
        recvRequest.push_back(rank);
        MPI_Get_count(&status,MPI_CHAR,&count);
        processRequest(rank,bufferRequest[rank],count);
      }
    }

    for(itRecv=recvRequest.begin();itRecv!=recvRequest.end();itRecv++)
    {
      pendingRequest.erase(*itRecv);
      bufferRequest.erase(*itRecv);
    }
  }

  void CLegacyContextServer::getBufferFromClient(size_t timeLine)
  {
    CTimer::get("CLegacyContextServer::getBufferFromClient").resume() ;

    int rank ;
    char *buffer ;
    size_t count ; 

    if (itLastTimeLine==lastTimeLine.end()) itLastTimeLine=lastTimeLine.begin() ;
    for(;itLastTimeLine!=lastTimeLine.end();++itLastTimeLine)
    {
      rank=itLastTimeLine->first ;
      if (itLastTimeLine->second < timeLine &&  pendingRequest.count(rank)==0 && buffers[rank]->isBufferEmpty())
      {
        if (buffers[rank]->getBufferFromClient(timeLine, buffer, count)) processRequest(rank, buffer, count);
        if (count >= 0) ++itLastTimeLine ;
        break ;
      }
    }
    CTimer::get("CLegacyContextServer::getBufferFromClient").suspend() ;
  }
         
       
  void CLegacyContextServer::processRequest(int rank, char* buff,int count)
  {

    CBufferIn buffer(buff,count);
    char* startBuffer,endBuffer;
    int size, offset;
    size_t timeLine=0;
    map<size_t,CEventServer*>::iterator it;

    
    CTimer::get("Process request").resume();
    while(count>0)
    {
      char* startBuffer=(char*)buffer.ptr();
      CBufferIn newBuffer(startBuffer,buffer.remain());
      newBuffer>>size>>timeLine;

      if (timeLine==timelineEventNotifyChangeBufferSize)
      {
        buffers[rank]->notifyBufferResizing() ;
        buffers[rank]->updateCurrentWindows() ;
        buffers[rank]->popBuffer(count) ;
        info(100)<<"Context id "<<context->getId()<<" : Receive NotifyChangeBufferSize from client rank "<<rank<<endl
                 <<"isBufferEmpty ? "<<buffers[rank]->isBufferEmpty()<<"  remaining count : "<<buffers[rank]->getUsed()<<endl;
      } 
      else if (timeLine==timelineEventChangeBufferSize)
      {
        size_t newSize ;
        vector<MPI_Aint> winBufferAdress(2) ;
        newBuffer>>newSize>>winBufferAdress[0]>>winBufferAdress[1] ;
        buffers[rank]->freeBuffer(count) ;
        delete buffers[rank] ;
        windows_[rank][0] -> setWinBufferAddress(winBufferAdress[0],0) ;
        windows_[rank][1] -> setWinBufferAddress(winBufferAdress[1],0) ;
        buffers[rank] = new CServerBuffer(rank, windows_[rank], winBufferAdress, 0, newSize) ;
        info(100)<<"Context id "<<context->getId()<<" : Receive ChangeBufferSize from client rank "<<rank
                 <<"  newSize : "<<newSize<<" Address : "<<winBufferAdress[0]<<" & "<<winBufferAdress[1]<<endl ;
      }
      else
      {
        info(100)<<"Context id "<<context->getId()<<" : Receive standard event from client rank "<<rank<<"  with timeLine : "<<timeLine<<endl ;
        it=events.find(timeLine);
       
        if (it==events.end()) it=events.insert(pair<int,CEventServer*>(timeLine,new CEventServer(this))).first;
        it->second->push(rank,buffers[rank],startBuffer,size);
        if (timeLine>0) lastTimeLine[rank]=timeLine ;
      }
      buffer.advance(size);
      count=buffer.remain();
    }
    
    CTimer::get("Process request").suspend();
  }

  void CLegacyContextServer::processEvents(bool enableEventsProcessing)
  {
    map<size_t,CEventServer*>::iterator it;
    CEventServer* event;
    
    if (isProcessingEvent_) return ;

    it=events.find(currentTimeLine);
    if (it!=events.end())
    {
      event=it->second;

      if (event->isFull())
      {
        if (!scheduled)
        {
          eventScheduler_->registerEvent(currentTimeLine,hashId);
          info(100)<<"Context id "<<context->getId()<<"Schedule event : "<< currentTimeLine <<"  "<<hashId<<endl ;
          scheduled=true;
        }
        else if (eventScheduler_->queryEvent(currentTimeLine,hashId) )
        {
          if (!enableEventsProcessing && isCollectiveEvent(*event)) return ;

          if (!eventScheduled_) 
          {
            MPI_Ibarrier(processEventBarrier_,&processEventRequest_) ;
            eventScheduled_=true ;
            return ;
          }
          else 
          {
            MPI_Status status ;
            int flag ;
            MPI_Test(&processEventRequest_, &flag, &status) ;
            if (!flag) return ;
            eventScheduled_=false ;
          }
          
          if (CXios::checkEventSync && context->getServiceType()!=CServicesManager::CLIENT)
          {
            int typeId, classId, typeId_in, classId_in;
            long long timeLine_out;
            long long timeLine_in( currentTimeLine );
            typeId_in=event->type ;
            classId_in=event->classId ;
   //        MPI_Allreduce(&timeLine,&timeLine_out, 1, MPI_UINT64_T, MPI_SUM, intraComm) ; // MPI_UINT64_T standardized by MPI 3
            MPI_Allreduce(&timeLine_in,&timeLine_out, 1, MPI_LONG_LONG_INT, MPI_SUM, intraComm) ; 
            MPI_Allreduce(&typeId_in,&typeId, 1, MPI_INT, MPI_SUM, intraComm) ;
            MPI_Allreduce(&classId_in,&classId, 1, MPI_INT, MPI_SUM, intraComm) ;
            if (typeId/intraCommSize!=event->type || classId/intraCommSize!=event->classId || timeLine_out/intraCommSize!=currentTimeLine)
            {
               ERROR("void CLegacyContextClient::sendEvent(CEventClient& event)",
                  << "Event are not coherent between client for timeline = "<<currentTimeLine);
            }
          }

          isProcessingEvent_=true ;
          CTimer::get("Process events").resume();
          info(100)<<"Context id "<<context->getId()<<" : Process Event "<<currentTimeLine<<" of class "<<event->classId<<" of type "<<event->type<<endl ;
          eventScheduler_->popEvent() ;
          dispatchEvent(*event);
          CTimer::get("Process events").suspend();
          isProcessingEvent_=false ;
          pendingEvent=false;
          delete event;
          events.erase(it);
          currentTimeLine++;
          scheduled = false;
        }
      }
      else if (pendingRequest.empty()) getBufferFromClient(currentTimeLine) ;
    }
    else if (pendingRequest.empty()) getBufferFromClient(currentTimeLine) ; // if pure one sided check buffer even if no event recorded at current time line
  }

  CLegacyContextServer::~CLegacyContextServer()
  {
    map<int,CServerBuffer*>::iterator it;
    for(it=buffers.begin();it!=buffers.end();++it) delete it->second;
    buffers.clear() ;
  }

  void CLegacyContextServer::releaseBuffers()
  {
    //for(auto it=buffers.begin();it!=buffers.end();++it) delete it->second ;
    //buffers.clear() ; 
    freeWindows() ;
  }

  void CLegacyContextServer::freeWindows()
  {
    for(auto& it : winComm_)
    {
      int rank = it.first ;
      delete windows_[rank][0];
      delete windows_[rank][1];
    }
  }

  void CLegacyContextServer::notifyClientsFinalize(void)
  {
    for(auto it=buffers.begin();it!=buffers.end();++it)
    {
      it->second->notifyClientFinalize() ;
    }
  }

  void CLegacyContextServer::dispatchEvent(CEventServer& event)
  {
    string contextName;
    string buff;
    int MsgSize;
    int rank;
    list<CEventServer::SSubEvent>::iterator it;
    StdString ctxId = context->getId();
    CContext::setCurrent(ctxId);
    StdSize totalBuf = 0;

    if (event.classId==CContext::GetType() && event.type==CContext::EVENT_ID_CONTEXT_FINALIZE)
    {
      finished=true;
      info(20)<<" CLegacyContextServer: Receive context <"<<context->getId()<<"> finalize."<<endl;
      notifyClientsFinalize() ;
      CTimer::get("receiving requests").suspend();
      context->finalize();
      
      std::map<int, StdSize>::const_iterator itbMap = mapBufferSize_.begin(),
                           iteMap = mapBufferSize_.end(), itMap;
      for (itMap = itbMap; itMap != iteMap; ++itMap)
      {
        rank = itMap->first;
        report(10)<< " Memory report : Context <"<<ctxId<<"> : server side : memory used for buffer of each connection to client" << endl
            << "  +) With client of rank " << rank << " : " << itMap->second << " bytes " << endl;
        totalBuf += itMap->second;
      }
      report(0)<< " Memory report : Context <"<<ctxId<<"> : server side : total memory used for buffer "<<totalBuf<<" bytes"<<endl;
    }
    else if (event.classId==CContext::GetType()) CContext::dispatchEvent(event);
    else if (event.classId==CContextGroup::GetType()) CContextGroup::dispatchEvent(event);
    else if (event.classId==CCalendarWrapper::GetType()) CCalendarWrapper::dispatchEvent(event);
    else if (event.classId==CDomain::GetType()) CDomain::dispatchEvent(event);
    else if (event.classId==CDomainGroup::GetType()) CDomainGroup::dispatchEvent(event);
    else if (event.classId==CAxis::GetType()) CAxis::dispatchEvent(event);
    else if (event.classId==CAxisGroup::GetType()) CAxisGroup::dispatchEvent(event);
    else if (event.classId==CScalar::GetType()) CScalar::dispatchEvent(event);
    else if (event.classId==CScalarGroup::GetType()) CScalarGroup::dispatchEvent(event);
    else if (event.classId==CGrid::GetType()) CGrid::dispatchEvent(event);
    else if (event.classId==CGridGroup::GetType()) CGridGroup::dispatchEvent(event);
    else if (event.classId==CField::GetType()) CField::dispatchEvent(event);
    else if (event.classId==CFieldGroup::GetType()) CFieldGroup::dispatchEvent(event);
    else if (event.classId==CFile::GetType()) CFile::dispatchEvent(event);
    else if (event.classId==CFileGroup::GetType()) CFileGroup::dispatchEvent(event);
    else if (event.classId==CVariable::GetType()) CVariable::dispatchEvent(event);
    else
    {
      ERROR("void CLegacyContextServer::dispatchEvent(CEventServer& event)",<<" Bad event class Id"<<endl);
    }
  }

  bool CLegacyContextServer::isCollectiveEvent(CEventServer& event)
  {
    if (event.type>1000) return false ;
    else return true ;
  }
}
