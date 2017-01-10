#include "xios_spl.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "buffer_out.hpp"
#include "buffer_client.hpp"
#include "cxios.hpp"
#include "mpi.hpp"
#include "tracer.hpp"

namespace xios
{
  size_t CClientBuffer::maxRequestSize = 0;

  CClientBuffer::CClientBuffer(MPI_Comm interComm, int serverRank, StdSize bufferSize, StdSize maxBufferedEvents)
    : interComm(interComm)
    , serverRank(serverRank)
    , bufferSize(bufferSize)
    , current(0)
    , count(0)
    , bufferedEvents(0)
    , maxBufferedEvents(maxBufferedEvents)
    , pending(false)
  {
    buffer[0] = new char[bufferSize]; // transform it with MPI_ALLOC_MEM later
    buffer[1] = new char[bufferSize];
    retBuffer = new CBufferOut(buffer[current], bufferSize);
    info(10) << "CClientBuffer: allocated 2 x " << bufferSize << " bytes for server " << serverRank << " with a maximum of " << maxBufferedEvents << " buffered events" << endl;
  }

  CClientBuffer::~CClientBuffer()
  {
   delete [] buffer[0];
   delete [] buffer[1];
   delete retBuffer;
  }

  int CClientBuffer::remain(void)
  {
    return bufferSize - count;
  }

  bool CClientBuffer::isBufferFree(int size)
  {
    if (size > maxRequestSize) maxRequestSize = size;

    if (size > bufferSize)
      ERROR("bool CClientBuffer::isBufferFree(int size)",
            << "The requested size (" << size << " bytes) is too big to fit the buffer (" << bufferSize << " bytes), please increase the client buffer size." << endl);

    return (size <= remain() && bufferedEvents < maxBufferedEvents);
  }


  CBufferOut* CClientBuffer::getBuffer(int size)
  {
    if (size <= remain())
    {
      retBuffer->realloc(buffer[current] + count, size);
      count += size;
      bufferedEvents++;
      return retBuffer;
    }
    else
    {
      ERROR("CBufferOut* CClientBuffer::getBuffer(int size)",
            << "Not enough space in buffer, this should not have happened...");
      return NULL;
    }
  }

  bool CClientBuffer::checkBuffer(void)
  {
    MPI_Status status;
    int flag;

    if (pending)
    {
      traceOff();
      MPI_Test(&request, &flag, &status);
      traceOn();
      if (flag == true) pending = false;
    }

    if (!pending)
    {
      if (count > 0)
      {
        MPI_Comm_test_inter(interComm, &flag);
        MPI_Issend(buffer[current], count, MPI_CHAR, serverRank, 20, interComm, &request);
        pending = true;
        if (current == 1) current = 0;
        else current = 1;
        count = 0;
        bufferedEvents = 0;
      }
    }

    return pending;
  }

  bool CClientBuffer::hasPendingRequest(void)
  {
    return (pending || count > 0);
  }
}
