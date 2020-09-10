#include "source_filter.hpp"
#include "grid.hpp"
#include "exception.hpp"
#include "calendar_util.hpp"
#include <limits> 

namespace xios
{
  CSourceFilter::CSourceFilter(CGarbageCollector& gc, CGrid* grid,
                               bool compression /*= true*/, bool mask /*= false*/,
                               const CDuration offset /*= NoneDu*/, bool manualTrigger /*= false*/,
                               bool hasMissingValue /*= false*/,
                               double defaultValue /*= 0.0*/)
    : COutputPin(gc, manualTrigger)
    , grid(grid)
    , compression(compression)
    , mask(mask)
    , offset(offset)
    , hasMissingValue(hasMissingValue), defaultValue(defaultValue)
  {
    if (!grid)
      ERROR("CSourceFilter::CSourceFilter(CGrid* grid)",
            "Impossible to construct a source filter without providing a grid.");
  }

  template <int N>
  void CSourceFilter::streamData(CDate date, const CArray<double, N>& data)
  {
    date = date + offset; // this is a temporary solution, it should be part of a proper temporal filter

    CDataPacketPtr packet(new CDataPacket);
    packet->date = date;
    packet->timestamp = date;
    packet->status = CDataPacket::NO_ERROR;

    packet->data.resize(grid->getStoreIndex_client().numElements());    
    
    if (compression)
    {
      packet->data = defaultValue;
      grid->uncompressField(data, packet->data);    
      
      // Convert missing values to NaN
      if (hasMissingValue) // probably to removed later
      {
        const double nanValue = std::numeric_limits<double>::quiet_NaN();
        const size_t nbData = packet->data.numElements();
        for (size_t idx = 0; idx < nbData; ++idx)
        {
          if (defaultValue == packet->data(idx))
          packet->data(idx) = nanValue;
        }
      }
    }
    else
    {
      if (mask) grid->maskField(data, packet->data);       // => ie coming from model
      //else grid->inputField(data, packet->data);
      else 
      {
        packet->data.resize(data.numElements()) ; // temporary solution, create own source filter for data coming from client
        CArray<double,1> tmp( (double*)data.dataFirst(),shape(data.numElements()),duplicateData) ;
        packet->data.reference(tmp) ;   // nothing to do if coming from client to server => workflow view
      }
    }

    onOutputReady(packet);
  }

  template void CSourceFilter::streamData<1>(CDate date, const CArray<double, 1>& data);
  template void CSourceFilter::streamData<2>(CDate date, const CArray<double, 2>& data);
  template void CSourceFilter::streamData<3>(CDate date, const CArray<double, 3>& data);
  template void CSourceFilter::streamData<4>(CDate date, const CArray<double, 4>& data);
  template void CSourceFilter::streamData<5>(CDate date, const CArray<double, 5>& data);
  template void CSourceFilter::streamData<6>(CDate date, const CArray<double, 6>& data);
  template void CSourceFilter::streamData<7>(CDate date, const CArray<double, 7>& data);

  void CSourceFilter::streamDataFromServer(CDate date, const std::map<int, CArray<double, 1> >& data)
  {
    date = date + offset; // this is a temporary solution, it should be part of a proper temporal filter

    CDataPacketPtr packet(new CDataPacket);
    packet->date = date;
    packet->timestamp = date;
    packet->status = CDataPacket::NO_ERROR;
    
    if (data.size() != grid->storeIndex_fromSrv_.size())
      ERROR("CSourceFilter::streamDataFromServer(CDate date, const std::map<int, CArray<double, 1> >& data)",
            << "Incoherent data received from servers,"
            << " expected " << grid->storeIndex_fromSrv_.size() << " chunks but " << data.size() << " were given.");

    packet->data.resize(grid->getStoreIndex_client().numElements());
    std::map<int, CArray<double, 1> >::const_iterator it, itEnd = data.end();
    for (it = data.begin(); it != itEnd; it++)
    {      
      CArray<int,1>& index = grid->storeIndex_fromSrv_[it->first];
      for (int n = 0; n < index.numElements(); n++)
        packet->data(index(n)) = it->second(n);
    }

    // Convert missing values to NaN
    if (hasMissingValue)
    {
      const double nanValue = std::numeric_limits<double>::quiet_NaN();
      const size_t nbData = packet->data.numElements();
      for (size_t idx = 0; idx < nbData; ++idx)
      {
        if (defaultValue == packet->data(idx))
          packet->data(idx) = nanValue;
      }
    }

    onOutputReady(packet);
  }

  void CSourceFilter::signalEndOfStream(CDate date)
  {
    date = date + offset; // this is a temporary solution, it should be part of a proper temporal filter

    CDataPacketPtr packet(new CDataPacket);
    packet->date = date;
    packet->timestamp = date;
    packet->status = CDataPacket::END_OF_STREAM;
    onOutputReady(packet);
  }
} // namespace xios
