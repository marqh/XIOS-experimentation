/*!
   \file generic_algorithm_transformation.hpp
   \author Ha NGUYEN
   \since 14 May 2015
   \date 29 June 2015

   \brief Interface for all transformation algorithms.
 */
#ifndef __XIOS_GENERIC_ALGORITHM_TRANSFORMATION_HPP__
#define __XIOS_GENERIC_ALGORITHM_TRANSFORMATION_HPP__

#include <map>
#include <set>
#include "array_new.hpp"
#include "client_client_dht_template.hpp"
#include "local_view.hpp"
#include "transform_connector.hpp"
#include "weight_transform_connector.hpp"

namespace xios
{
  class CGrid;
  class CDomain;
  class CAxis;
  class CScalar;
  class CGridAlgorithm ;
  class CTransformFilter ;
  class CGarbageCollector ;

  /*!
  \class CGenericAlgorithmTransformation
  This class defines the interface for all other inherited algorithms class
  */
class CGenericAlgorithmTransformation : public std::enable_shared_from_this<CGenericAlgorithmTransformation>
{
  public : 
    CGenericAlgorithmTransformation(bool isSource) ;
    virtual shared_ptr<CGridAlgorithm> createGridAlgorithm(CGrid* gridSrc, CGrid* newGrid, int pos) ;
    virtual CTransformFilter* createTransformFilter(CGarbageCollector& gc, shared_ptr<CGridAlgorithm> algo, bool detectMissingValues, double defaultValue) ;
    virtual void apply(int dimBefore, int dimAfter, const CArray<double,1>& dataIn, CArray<double,1>& dataOut) { abort() ;} //=0
    virtual void apply(int dimBefore, int dimAfter, const CArray<double,1>& dataIn, const vector<CArray<double,1>>& auxData, CArray<double,1>& dataOut) { abort() ;} //=0
    virtual bool isGenerateTransformation(void) { return false ;}
    virtual StdString getAlgoName() { return "\\nGeneric algorithm transformation";}
    virtual bool transformAuxField(int pos) { return true ;}
    virtual vector<string> getAuxFieldId(void) ;
  protected :
    typedef std::unordered_map<int, std::vector<int> > TransformationIndexMap;
    typedef std::unordered_map<int, std::vector<double> > TransformationWeightMap;
 
    shared_ptr<CLocalElement> recvElement_ ;
    bool isSource_ ;

  public:
    shared_ptr<CLocalElement> getRecvElement(void) { return recvElement_ ;}
  
};

}
#endif // __XIOS_GENERIC_ALGORITHM_TRANSFORMATION_HPP__
