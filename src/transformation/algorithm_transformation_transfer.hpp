#ifndef __XIOS_ALGORITHM_TRANSFORMATION_TRANSFER_HPP__
#define __XIOS_ALGORITHM_TRANSFORMATION_TRANSFER_HPP__

#include "generic_algorithm_transformation.hpp"
#include "transfer_transform_connector.hpp"
#include "array_new.hpp"

namespace xios
{

  class CAlgorithmTransformationTransfer : public CGenericAlgorithmTransformation
  {
    public:

      CAlgorithmTransformationTransfer(bool isSource) : CGenericAlgorithmTransformation(isSource) {}
      virtual ~CAlgorithmTransformationTransfer() {};
      virtual void apply(int dimBefore, int dimAfter, const CArray<double,1>& dataIn, CArray<double,1>& dataOut);
      virtual void computeRecvElement(CLocalView* srcView, CLocalView* dstView) ;
   
    protected:
      virtual void computeAlgorithm(CLocalView* srcView, CLocalView* dstView) ;

      //! Map between global index of destination element and source element
      unordered_map<int,int> transformationMapping_;
      CTransferTransformConnector* transferTransformConnector_ ;
  };

}
#endif //__XIOS_ALGORITHM_TRANSFORMATION_TRANSFER_HPP__
