/*!
   \file grid_transformation_factory_impl.hpp
   \author Ha NGUYEN
   \since 23 June 2016
   \date 23 June 2016

   \brief Helper class to create different transformations.
 */
#ifndef __XIOS_GRID_TRANSFORMATION_FACTORY_HPP__
#define __XIOS_GRID_TRANSFORMATION_FACTORY_HPP__

#include "xios_spl.hpp"
#include "exception.hpp"
#include "transformation.hpp"

namespace xios {

class CGrid;
class CGenericAlgorithmTransformation;


/*!
  \class CGridTransformationFactory
  This class is a helper class to chose a algorithm (transformation) from the alogrithm list of
specific grid.
*/
template<typename T>
class CGridTransformationFactory
{
public:
  /** Default constructor */
  CGridTransformationFactory() {}
  virtual ~CGridTransformationFactory() {}

  static shared_ptr<CGenericAlgorithmTransformation> createTransformation(ETranformationType transType, bool isSource,
                                                               CGrid* gridDst, CGrid* gridSrc,
                                                               CTransformation<T>* transformation,
                                                               int elementPositionInGrid,
                                                               std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                               std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                               std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                               std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                               std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                               std::map<int, int>& elementPositionInGridDst2DomainPosition);

public:
  typedef shared_ptr<CGenericAlgorithmTransformation> (*CreateTransformationCallBack)(bool isSource, CGrid* gridDst, CGrid* gridSrc,
                                                                           CTransformation<T>* transformation,
                                                                           int elementPositionInGrid,
                                                                           std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                                           std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                                           std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                                           std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                                           std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                                           std::map<int, int>& elementPositionInGridDst2DomainPosition);

  typedef std::map<ETranformationType, CreateTransformationCallBack> CallBackMap;
  static CallBackMap* transformationCreationCallBacks_;
  static bool registerTransformation(ETranformationType transType, CreateTransformationCallBack createFn);
  static bool unregisterTransformation(ETranformationType transType);
  static bool initializeTransformation_;
  static void unregisterAllTransformations(void) ;
};

template<typename T>
typename CGridTransformationFactory<T>::CallBackMap* CGridTransformationFactory<T>::transformationCreationCallBacks_ = 0;
template<typename T>
bool CGridTransformationFactory<T>::initializeTransformation_ = false;

template<typename T>
shared_ptr<CGenericAlgorithmTransformation> CGridTransformationFactory<T>::createTransformation(ETranformationType transType, bool isSource,
                                                                               CGrid* gridDst, CGrid* gridSrc,
                                                                               CTransformation<T>* transformation,
                                                                               int elementPositionInGrid,
                                                                               std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                                               std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                                               std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                                               std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                                               std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                                               std::map<int, int>& elementPositionInGridDst2DomainPosition)
{
  typename CallBackMap::const_iterator it = (*transformationCreationCallBacks_).find(transType);
  if ((*transformationCreationCallBacks_).end() == it)
  {
     ERROR("CGridTransformationFactory::createTransformation(ETranformationType transType)",
           << "Transformation type " << transType
           << "doesn't exist. Please define.");
  }
  return (it->second)(isSource, gridDst, gridSrc, transformation, elementPositionInGrid,
                      elementPositionInGridSrc2ScalarPosition,
                      elementPositionInGridSrc2AxisPosition,
                      elementPositionInGridSrc2DomainPosition,
                      elementPositionInGridDst2ScalarPosition,
                      elementPositionInGridDst2AxisPosition,
                      elementPositionInGridDst2DomainPosition);
}

template<typename T>
bool CGridTransformationFactory<T>::registerTransformation(ETranformationType transType, CreateTransformationCallBack createFn)
{
  if (nullptr == transformationCreationCallBacks_)
    transformationCreationCallBacks_ = new CallBackMap();

  return (*transformationCreationCallBacks_).insert(make_pair(transType, createFn)).second;
}

template<typename T>
bool CGridTransformationFactory<T>::unregisterTransformation(ETranformationType transType)
{
  return (1 == (*transformationCreationCallBacks_).erase(transType));
}

template<typename T>
void CGridTransformationFactory<T>::unregisterAllTransformations(void)
{
  if (nullptr != transformationCreationCallBacks_) 
  {
    transformationCreationCallBacks_->clear() ;
    delete transformationCreationCallBacks_;
  }
}

}

#endif // __XIOS_GRID_TRANSFORMATION_FACTORY_HPP__
