#ifndef __XIOS_ALGORITHM_TRANSFORMATION_WEIGHT_HPP__
#define __XIOS_ALGORITHM_TRANSFORMATION_WEIGHT_HPP__

#include "generic_algorithm_transformation.hpp"
#include "array_new.hpp"
#include "local_view.hpp"
#include "transform_connector.hpp"
#include "weight_transform_connector.hpp"

namespace xios
{

  class CAlgorithmTransformationWeight : public CGenericAlgorithmTransformation
  {
    public:

      CAlgorithmTransformationWeight(bool isSource) : CGenericAlgorithmTransformation(isSource) {}
      virtual ~CAlgorithmTransformationWeight() {};
      virtual void apply(int dimBefore, int dimAfter, const CArray<double,1>& dataIn, CArray<double,1>& dataOut);
      virtual void computeRecvElement(CLocalView* srcView, CLocalView* dstView);
    protected:
      virtual void computeAlgorithm(CLocalView* srcView, CLocalView* dstView) ;

      //! Map between global index of destination element and source element
      TransformationIndexMap transformationMapping_;
      //! Weight corresponding of source to destination
      TransformationWeightMap transformationWeight_;
      CWeightTransformConnector* weightTransformConnector_ ;
      
  };

}
#endif //__XIOS_ALGORITHM_TRANSFORMATION_WEIGHT_HPP__
