/*!
   \file axis_algorithm_reduce_axis.cpp
   \author Ha NGUYEN
   \since 23 June 2016
   \date 23 June 2016

   \brief Algorithm for reduce a axis to an axis
 */
#include "axis_algorithm_reduce_axis.hpp"
#include "reduce_axis_to_axis.hpp"
#include "axis.hpp"
#include "grid.hpp"
#include "grid_transformation_factory_impl.hpp"
#include "grid_algorithm_reduce.hpp"


namespace xios {
shared_ptr<CGenericAlgorithmTransformation> CAxisAlgorithmReduceAxis::create(bool isSource, CGrid* gridDst, CGrid* gridSrc,
                                                                   CTransformation<CAxis>* transformation,
                                                                   int elementPositionInGrid,
                                                                   std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                                   std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                                   std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                                   std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                                   std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                                   std::map<int, int>& elementPositionInGridDst2DomainPosition)
TRY
{
  std::vector<CAxis*> axisListDestP = gridDst->getAxis();
  std::vector<CAxis*> axisListSrcP  = gridSrc->getAxis();

  CReduceAxisToAxis* reduceAxis = dynamic_cast<CReduceAxisToAxis*> (transformation);
  int axisDstIndex   = elementPositionInGridDst2AxisPosition[elementPositionInGrid];
  int axisSrcIndex = elementPositionInGridSrc2AxisPosition[elementPositionInGrid];

  return make_shared<CAxisAlgorithmReduceAxis>(isSource, axisListDestP[axisDstIndex], axisListSrcP[axisSrcIndex], reduceAxis);
}
CATCH

bool CAxisAlgorithmReduceAxis::dummyRegistered_ = CAxisAlgorithmReduceAxis::registerTrans();
bool CAxisAlgorithmReduceAxis::registerTrans()
TRY
{
  return CGridTransformationFactory<CAxis>::registerTransformation(TRANS_REDUCE_AXIS_TO_AXIS, create);
}
CATCH


CAxisAlgorithmReduceAxis::CAxisAlgorithmReduceAxis(bool isSource, CAxis* axisDestination, CAxis* axisSource, CReduceAxisToAxis* algo)
 : CAlgorithmTransformationReduce(isSource)
TRY
{
  if (!axisDestination->checkGeometricAttributes(false))
  {
    axisDestination->resetGeometricAttributes();
    axisDestination->setGeometricAttributes(*axisSource) ;
  }
  axisDestination->checkAttributes() ; 
  algo->checkValid(axisDestination, axisSource);
  

  switch (algo->operation)
  {
    case CReduceAxisToAxis::operation_attr::sum:
      operator_ = EReduction::sum;
      break;
    case CReduceAxisToAxis::operation_attr::min:
      operator_ = EReduction::min;
      break;
    case CReduceAxisToAxis::operation_attr::max:
      operator_ = EReduction::max;
      break;
    case CReduceAxisToAxis::operation_attr::average:
      operator_ = EReduction::average;
      break;
    default:
        ERROR("CAxisAlgorithmReduceAxis::CAxisAlgorithmReduceAxis(CAxis* axisDestination, CAxis* axisSource, CReduceAxisToAxis* algo)",
         << "Operation is wrongly defined. Supported operations: sum, min, max, average." << std::endl
         << "Axis source " <<axisSource->getId() << std::endl
         << "Axis destination " << axisDestination->getId());
  }

  //TransformationIndexMap& transMap = this->transformationMapping_;
  //CArray<int,1>& axisDstIndex = axisDestination->index;
  //int nbAxisIdx = axisDstIndex.numElements();



  auto& transMap = this->transformationMapping_;
  
  CArray<size_t,1> dstGlobalIndex ;
  axisDestination->getLocalView(CElementView::WORKFLOW)->getGlobalIndexView(dstGlobalIndex) ;
  size_t nbIdx = dstGlobalIndex.numElements();

  for (size_t idx = 0; idx < nbIdx; ++idx)
  {
    size_t globalIdx = dstGlobalIndex(idx);
    transMap[globalIdx].resize(1);
    transMap[globalIdx][0]=globalIdx ;      
  }

  this->computeAlgorithm(axisSource->getLocalView(CElementView::WORKFLOW), axisDestination->getLocalView(CElementView::WORKFLOW)) ;
}
CATCH

shared_ptr<CGridAlgorithm> CAxisAlgorithmReduceAxis::createGridAlgorithm(CGrid* gridSrc, CGrid* gridDst, int pos)
{
  auto algo=make_shared<CGridAlgorithmReduce>(gridSrc, gridDst, pos, shared_from_this(), operator_) ;
  algo->computeAlgorithm(false) ;
  return algo ; 
}

CAxisAlgorithmReduceAxis::~CAxisAlgorithmReduceAxis()
TRY
{
 
}
CATCH


}
