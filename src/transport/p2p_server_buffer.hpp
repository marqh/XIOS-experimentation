#ifndef __P2P_SERVER_BUFFER_HPP__
#define __P2P_SERVER_BUFFER_HPP__

#include "xios_spl.hpp"
#include "buffer_out.hpp"
#include "mpi.hpp"
#include "cxios.hpp"
#include "event_server.hpp"
#include "p2p_cs_buffer_base.hpp"
#include "p2p_server_base.hpp"

namespace xios
{
  extern CLogType logProtocol ;

  class CP2pServerBuffer : public CP2pCSBufferBase, public CP2pServerBase
  {
    private :

      class CBuffer
      {
        char* buffer_ ;
        size_t start_ ;
        size_t end_ ;
        size_t count_ ;
        size_t size_ ;
        bool fixed_ ;

        public:
          CBuffer(size_t size, bool fixed) : start_(size), end_(0), count_(0), size_(size), fixed_(fixed) 
          { 
            MPI_Alloc_mem(size, MPI_INFO_NULL, &buffer_) ;
            info(logProtocol)<<"New buffer of size="<<size<<endl ;
          }
          ~CBuffer() 
          { 
            if (count_>0) ERROR("COneSidedServerBuffer::~CBuffer()",<<"Try to delete buffer that is not empty"<<std::endl) ;
            MPI_Free_mem(buffer_) ;
          }
       
          void reserve(size_t& size, size_t& start, size_t& count)
          {
            count = 0 ;
            if (end_ < start_)
            {
              if (start_-end_ >= size)
              {
                //if (start!=end_) info(logProtocol)<<" CASE 0, recv/computed : " << start << "/" << end_ << endl;
                count=size ;
                size = 0 ;
                start=end_ ;
                end_  += count ; 
                count_+= count ;
              } 
              else
              {
		// Server buffers get blocs from clients, they should not be splitted (mirrors the client buffers)
                //if (start!=end_) info(logProtocol)<<" CASE 1"<< endl;
                //info(logProtocol)<<" CASE 1 : start/end/count - size : "<< start_ << ", " << end_ << ", " << count_ << " " << size_ << " vs size " << size << endl;
                ERROR("COneSidedServerBuffer::reserve()",<<"This should be the case of the 2nd part of a splitted message"<<std::endl);
                //count = start_-end_ ;
                //size -= count ;
                //start=end_ ;
                //end_ = start_ ;
                //count_+=count ;
                count = 0 ; 
              }
            }
            else if ( end_> start_ )
            {
              if (size_-end_ >= size)
              {
                //if (start!=end_ ) info(logProtocol)<<" CASE 2, recv/computed : "<< start << "/" << end_ << endl;
                count = size ;
                size = 0;
                start=end_ ;
                end_   += count ;
                count_ += count ;
              }
              else
              {
                // Server buffers get blocs from clients, they should not be splitted (mirrors the client buffers)
                //     1st part of a splitted message, fill the end of the buffer
                //     end_ must be set to 0 like on clients
                //if (start!=end_) info(logProtocol)<<" CASE 3"<< endl;
                //  info(logProtocol)<<" CASE 3 : start/end/count - size : "<< start_ << ", " << end_ << ", " << count_ << " " << size_ << " vs size " << size << endl;
                //count = size_ - end_ ;
                //size -= count ;
                //start=end_ ;
                end_ = 0 ;
                //count_+= count ;
                count = 0 ;
              }
            }
            else if (end_==start_)
            {
              count = 0 ;
            }
          }

          void free(size_t start, size_t count)
          {
            start_ = start+count-1 ;
            count_ -= count ;
          }

          size_t remain(void) {
            if (count_==0)
              return size_;
            else if (end_<start_)
              return start_-end_;
            else
              return size_-end_;
          }
          size_t getSize(void) { return size_ ;}
          size_t getCount(void) {return count_ ;}
          size_t isFixed(void) {return fixed_;}
          char* getBuffer(void) {return buffer_ ;}
      } ;
     
    public:
    
      CP2pServerBuffer(int clientRank, const MPI_Comm& commSelf, const MPI_Comm& interCommMerged, map<size_t, SPendingEvent>& pendingEvents, 
                             map<size_t, SPendingEvent>& completedEvents, vector<char>& buffer)  ;
      
      ~CP2pServerBuffer()
      {
          while (!buffers_.empty()) {
              delete buffers_.front();
              buffers_.erase(buffers_.begin()) ; // if buffer is empty free buffer
              countDeletedBuffers_++;
          }
      };

      void receivedRequest(vector<char>& buffer) ;
      void eventLoop(void) ;
      void fillEventServer(size_t timeline, CEventServer& event) ;
      void notifyClientFinalize(void);

    private:
      struct SBloc
      {
        CBuffer* buffer ;
        size_t start ;
        int count ;
        MPI_Aint addr ;
      } ;

      void createWindow(const MPI_Comm& commSelf, const MPI_Comm& interCommMerged) ;
      void newBuffer(size_t size, bool fixed) { buffers_.push_back(new CBuffer(size, fixed)); currentBuffer_=buffers_.back() ;}
      void testPendingRequests(void) ;
      void transferEvents(void) ;
      void transferEvent(void) ;
      void transferRmaRequest(size_t timeline, MPI_Aint addr, MPI_Aint offset, CBuffer* buffer, size_t start, int count, int window) ;
      size_t remainSize(void) ;
      size_t remainSize(int bufferId) ;



      bool fixed_=false;
      size_t fixedSize_ = 0 ;
      size_t currentBufferSize_=0 ;
      double growingFactor_ = 1. ;
      double bufferServerFactor_=1. ;
      
      std::vector<CBuffer*> buffers_ ;
      CBuffer* currentBuffer_=nullptr ;

      map<size_t, SPendingEvent>& pendingFullEvents_ ;
      map<size_t, SPendingEvent>& completedFullEvents_ ;

      map<size_t, int> nbSenders_ ;
      map<size_t, list<tuple<MPI_Aint,int,int,size_t>>> pendingBlocs_;
     
      vector<MPI_Request> pendingRmaRequests_ ;
      vector<MPI_Status> pendingRmaStatus_ ;
      vector<char*> pendingRmaAddr_ ;
      vector<int> pendingRmaCount_ ;

      map<size_t, list<SBloc>> onTransferEvents_ ; // map<size_t timeline, list<pair<char* bloc, int count>>>
      map<size_t, list<SBloc>> completedEvents_ ; // map<size_t timeline, list<pair<char* bloc, int count>>>
      list<std::pair<size_t,size_t>> bufferResize_ ; // list<size_t AssociatedTimeline, size_t newSize> 

      int clientRank_ ;
      MPI_Aint * control_ ;
      MPI_Aint controlAddr_ ;

      MPI_Comm interCommMerged_ ;
      MPI_Comm winComm_ ;
      vector<MPI_Win> windows_ ;
      int maxWindows_ ;
      set<int> windowsLocked_ ;

      MPI_Win winControl_ ;
      //bool isLocked_=false ;
      const int windowRank_=0 ;
      MPI_Aint lastBlocToFree_=0 ;
      int countDeletedBuffers_;

  } ;

}


#endif
