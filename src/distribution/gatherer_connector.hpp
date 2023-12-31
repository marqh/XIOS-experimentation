#ifndef __GATHERER_CONNECTOR_HPP__
#define __GATHERER_CONNECTOR_HPP__

#include "xios_spl.hpp"
#include "array_new.hpp"
#include "distributed_view.hpp"
#include "mpi.hpp"
#include "local_view.hpp"
#include "distributed_view.hpp"
#include "context_client.hpp"
#include "reduction_types.hpp"


namespace xios
{
 
  class CGathererConnector
  {
    private:
      shared_ptr<CDistributedView> srcView_;
      shared_ptr<CLocalView> dstView_;
      map<int, vector<int>> connector_ ;
      map<int, vector<bool>> mask_ ;  // mask is on src view
      int dstSize_ ; 
      map<int,int> srcSize_ ;

    public:
      CGathererConnector(shared_ptr<CDistributedView> srcView, shared_ptr<CLocalView> dstView) : srcView_(srcView), dstView_(dstView) {} ;
      void computeConnector(void) ;
      
      template<typename T>
      void transfer(int repeat, int sizeT, map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, EReduction op = EReduction::none)
      {
        // for future, make a specific transfer function for sizeT=1 to avoid multiplication (increasing performance)
       
        size_t dstSlice = dstSize_*sizeT ;
        dataOut.resize(repeat* dstSlice) ;  
        
        
        if (op == EReduction::none) // tranfer without reduction
        {
          for(auto& data : dataIn)
          {
            T* output = dataOut.dataFirst() ;
            int rank=data.first ;
            auto input  = data.second.dataFirst() ;
            auto& connector=connector_[rank] ;
            auto& mask=mask_[rank] ;
            int size=mask.size() ;
            size_t srcSlice = size * sizeT ;
            for(int l=0; l<repeat; l++)
            {   
              for(int i=0, j=0 ;i<size;i++)
              {
                if (mask[i]) 
                {
                  int cj = connector[j]*sizeT ;
                  int ci = i*sizeT ;
                  for (int k=0;k<sizeT;k++) output[cj+k] = input[ci+k] ;
                  j++ ;
                }
              }
              input+=srcSlice ;
              output+=dstSlice ;
           }
          }
        }
        else // with reduction, to be optimized, see how to vectorize
        {
          vector<int> vcount(dataOut.size(),0) ;
          int* count = vcount.data() ;
          T defaultValue = std::numeric_limits<T>::quiet_NaN();
          for(auto& data : dataIn)
          {
            T* output = dataOut.dataFirst() ;
            int rank=data.first ;
            auto input  = data.second.dataFirst() ;
            auto& connector=connector_[rank] ;
            auto& mask=mask_[rank] ;
            int size=mask.size() ;
            size_t srcSlice = size * sizeT ;
            for(int l=0; l<repeat; l++)
            {   
              for(int i=0, j=0 ;i<size;i++)
              {
                if (mask[i]) 
                {
                  int cj = connector[j]*sizeT ;
                  int ci = i*sizeT ;
                  for (int k=0;k<sizeT;k++)
                  { 
                    if (!std::isnan(input[ci+k])) // manage missing value
                    {
                      if (count[cj+k]==0)  output[cj+k] = input[ci+k] ;
                      else
                      {
                        switch(op)
                        {
                          case EReduction::sum :
                            output[cj+k] += input[ci+k] ;
                            break ;
                          case EReduction::min :
                            output[cj+k]= std::min(output[cj+k],input[ci+k]) ;
                            break ;
                          case EReduction::max :
                            output[cj+k]= std::max(output[cj+k],input[ci+k]) ;
                            break ;
                          case EReduction::average :
                            output[cj+k] += input[ci+k] ;
                            break ;
                          default :
                            ERROR("CGathererConnector::transfer",
                               <<"reduction operator "<<(int)op<<" is not defined for this operation") ;
                          break ;
                        }
                      }
                      count[cj+k]++ ; 
                    }
                  }
                  j++ ;
                }
              }
              input+=srcSlice ;
              output+=dstSlice ;
              count+=dstSlice ;
            }
          }

          T* output = dataOut.dataFirst() ;
          if (op==EReduction::average)
            for(int i=0;  i < dataOut.size() ; i++) 
            {
              if (count[i]>0) dataOut[i]/=count[i] ;
              else dataOut[i] = defaultValue ;
            }
          else for(int i=0;  i < dataOut.size() ; i++) if (count[i]==0) dataOut[i] = defaultValue ;
        }
      }

      template<typename T>
      void transfer(int sizeT, map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, EReduction op = EReduction::none)
      {
        transfer(1, sizeT, dataIn, dataOut, op) ;
      }
    
      template<typename T>
      void transfer(map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, EReduction op = EReduction::none)
      {
        transfer(1,dataIn,dataOut, op) ;
      }

      template<typename T>
      void transfer(int rank,  shared_ptr<CGathererConnector>* connectors, int nConnectors, const T* input, T* output, EReduction op = EReduction::none, int* count=nullptr)
      {
        auto& connector = connector_[rank] ; // probably costly, find a better way to avoid the map
        auto& mask = mask_[rank] ; 
        int srcSize = mask.size() ;
      
        if (nConnectors==0)
        {
          if (op == EReduction::none)
          {
            for(int i=0, j=0; i<srcSize; i++)
              if (mask[i]) 
              {
                *(output+connector[j]) = *(input + i) ;
                j++ ;
              }
          }
          else
          {
            switch(op)
            {
              case EReduction::sum :
                for(int i=0, j=0; i<srcSize; i++)
                  if (mask[i]) 
                  {
                    if (!std::isnan(*(input + i)))
                    {
                      if (*(count+connector[j])==0) *(output+connector[j]) = *(input + i) ;
                      else *(output+connector[j]) += *(input + i) ;
                      (*(count+connector[j]))++ ;
                    } 
                    j++ ;
                  }
                break ;
              case EReduction::min :
                for(int i=0, j=0; i<srcSize; i++)
                  if (mask[i]) 
                  {
                    if (!std::isnan(*(input + i)))
                    {
                      if (*(count+connector[j])==0) *(output+connector[j]) = *(input + i) ;
                      else *(output+connector[j]) = std::min(*(output+connector[j]),*(input + i)) ;
                      (*(count+connector[j]))++ ;
                    } 
                    j++ ;
                  }
                break ;
              case EReduction::max :
                for(int i=0, j=0; i<srcSize; i++)
                  if (mask[i]) 
                  {
                    if (!std::isnan(*(input + i)))
                    {
                      if (*(count+connector[j])==0) *(output+connector[j]) = *(input + i) ;
                      else *(output+connector[j]) = std::max(*(output+connector[j]),*(input + i)) ;
                      (*(count+connector[j]))++ ;
                    } 
                    j++ ;
                  }
                break ;
              case EReduction::average :
                for(int i=0, j=0; i<srcSize; i++)
                  if (mask[i]) 
                  {
                    if (!std::isnan(*(input + i)))
                    {
                      if (*(count+connector[j])==0) *(output+connector[j]) = *(input + i) ;
                      else *(output+connector[j]) = *(output+connector[j])* (*(count+connector[j])) + *(input + i) ;
                      (*(count+connector[j]))++ ;
                    } 
                    j++ ;
                  }
                for(int i=0, j=0; i<srcSize; i++)
                    if (mask[i]) 
                    {
                      if (!std::isnan(*(input + i)))
                        *(output+connector[j]) /= (*(count+connector[j]));
                      j++ ;
                    }
                break ;
              default :
                ERROR("CGathererConnector::transfer",
                     <<"reduction operator "<<(int)op<<" is not defined for this operation") ;
                break ;
            }
          }

        }
        else
       {
          int srcSliceSize = (*(connectors-1))->getSrcSliceSize(rank, connectors-1, nConnectors-1) ;
          int dstSliceSize = (*(connectors-1))->getDstSliceSize(connectors-1, nConnectors-1) ;

          const T* in = input ; 
          for(int i=0,j=0;i<srcSize;i++) 
          {
            if (mask[i]) 
            {
              (*(connectors-1))->transfer(rank, connectors-1, nConnectors-1, in, output+connector[j]*dstSliceSize, op, count+connector[j]*dstSliceSize) ; // the multiplication must be avoid in further optimization
              j++ ;
            }
            in += srcSliceSize ;
          }
        }

      }

      // hook for transfering mask in grid connector, maybe find an other way to doing that...
      void transfer_or(int rank,  shared_ptr<CGathererConnector>* connectors, int nConnectors, const bool* input, bool* output)
      {
        auto& connector = connector_[rank] ; // probably costly, find a better way to avoid the map
        auto& mask = mask_[rank] ; 
        int srcSize = mask.size() ;
      
        if (nConnectors==0)
        {
          for(int i=0, j=0; i<srcSize; i++)
            if (mask[i]) 
            {
              *(output+connector[j]) |= *(input + i) ;
              j++ ;
            }

        }
        else
       {
          int srcSliceSize = (*(connectors-1))->getSrcSliceSize(rank, connectors-1, nConnectors-1) ;
          int dstSliceSize = (*(connectors-1))->getDstSliceSize(connectors-1, nConnectors-1) ;

          const bool* in = input ; 
          for(int i=0,j=0;i<srcSize;i++) 
          {
            if (mask[i]) 
            {
              (*(connectors-1))->transfer_or(rank, connectors-1, nConnectors-1, in, output+connector[j]*dstSliceSize) ; // the multiplication must be avoid in further optimization
              j++ ;
            }
            in += srcSliceSize ;
          }
        }

      }



      template<typename T>
      void transfer(map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, T missingValue, EReduction op = EReduction::none)
      {
        transfer(1, 1, dataIn, dataOut, missingValue, op);
      }
      
      template<typename T>
      void transfer(int sizeT, map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, T missingValue, EReduction op = EReduction::none)
      {
         transfer(1, sizeT, dataIn, dataOut, missingValue, op) ;
      }

      template<typename T>
      void transfer(int repeat , int sizeT, map<int, CArray<T,1>>& dataIn, CArray<T,1>& dataOut, T missingValue, EReduction op = EReduction::none)
      {
        dataOut.resize(repeat*dstSize_*sizeT) ;
        dataOut=missingValue ;
        transfer(repeat, sizeT, dataIn, dataOut, op) ;
      }

      template<typename T>
      void transfer(CEventServer& event, int sizeT, CArray<T,1>& dataOut, EReduction op = EReduction::none)
      {
        map<int, CArray<T,1>> dataIn ;
        for (auto& subEvent : event.subEvents) 
        {
          auto& data = dataIn[subEvent.rank]; 
          (*subEvent.buffer) >> data ;
        }
        transfer(1, sizeT, dataIn, dataOut, op) ;
      }
      
      template<typename T>
      void transfer(CEventServer& event, CArray<T,1>& dataOut, EReduction op = EReduction::none)
      {
        transfer(event, 1, dataOut, op) ;
      }

      template<typename T>
      void transfer(CEventServer& event, int sizeT, CArray<T,1>& dataOut, T missingValue, EReduction op = EReduction::none)
      {
        map<int, CArray<T,1>> dataIn ;
        for (auto& subEvent : event.subEvents) 
        {
          auto& data = dataIn[subEvent.rank]; 
          (*subEvent.buffer) >> data ;
        }
        transfer(1, sizeT, dataIn, dataOut, missingValue, op) ;
      }

      template<typename T>
      void transfer(CEventServer& event, CArray<T,1>& dataOut, T missingValue, EReduction op = EReduction::none)
      {
        map<int, CArray<T,1>> dataIn ;
        for (auto& subEvent : event.subEvents) 
        {
          auto& data = dataIn[subEvent.rank]; 
          (*subEvent.buffer) >> data ;
        }
        transfer(1, 1, dataIn, dataOut, missingValue, op) ;
      }

    int getSrcSliceSize(int rank, shared_ptr<CGathererConnector>* connectors, int nConnectors) 
    { if (nConnectors==0) return srcSize_[rank] ; else return srcSize_[rank] * (*(connectors-1))->getSrcSliceSize(rank, connectors-1,nConnectors-1) ; }

    int getDstSliceSize(shared_ptr<CGathererConnector>* connectors, int nConnectors) 
    { if (nConnectors==0) return dstSize_ ; else return dstSize_ * (*(connectors-1))->getDstSliceSize(connectors-1,nConnectors-1) ; }
  
    int getDstSize(void) {return dstSize_ ;}
  } ;

}

#endif
