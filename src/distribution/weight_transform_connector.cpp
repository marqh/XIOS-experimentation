#include "weight_transform_connector.hpp"

namespace xios
{

  CWeightTransformConnector::CWeightTransformConnector(shared_ptr<CLocalView> srcView, shared_ptr<CLocalView> dstView, unordered_map<int, std::vector<int>>& indexMap, 
                                                       unordered_map<int, std::vector<double>>& weightMap,  bool detectMissingValue, bool renormalize) : 
                                                       srcView_(srcView), dstView_(dstView), detectMissingValue_(detectMissingValue), renormalize_(renormalize)
  {
    computeConnector(indexMap, weightMap) ;
  }

  void CWeightTransformConnector::computeConnector(unordered_map<int, std::vector<int>>& indexMap, 
                                                   unordered_map<int, std::vector<double>>& weightMap)
  {
    CArray<size_t,1> dstGlobalIndex ;
    CArray<size_t,1> srcGlobalIndex ;
    dstView_->getGlobalIndexView(dstGlobalIndex) ;
    srcView_->getGlobalIndexView(srcGlobalIndex) ;
    unordered_map<size_t,int> srcMapIndex ;
    srcSize_ = srcGlobalIndex.numElements() ;
    dstSize_ = dstGlobalIndex.numElements() ;

    for(int i=0;i<srcSize_;i++) srcMapIndex[srcGlobalIndex(i)]=i ;
    for(int i=0; i< dstSize_;i++) 
    {
      if (indexMap.count(dstGlobalIndex(i))!=0 && weightMap.count(dstGlobalIndex(i))!=0)
      {
        auto& vectIndex  = indexMap[dstGlobalIndex(i)] ;
        auto& vectWeight = weightMap[dstGlobalIndex(i)] ;
        nWeights_.push_back(vectIndex.size()) ;
        for(int j=0; j<vectIndex.size();j++)
        {
          connector_.push_back(srcMapIndex[vectIndex[j]]) ;
          weights_.push_back(vectWeight[j]) ;
        }
      }
      else nWeights_.push_back(0) ;
    }
  }

}