/*!
   \file scalar_algorithm_reduce_scalar.hpp
   \author Ha NGUYEN
   \since 23 June 2016
   \date 23 June 2016

   \brief Algorithm for reduce an axis to a scalar
 */
#ifndef __XIOS_SCALAR_ALGORITHM_REDUCE_AXIS_HPP__
#define __XIOS_SCALAR_ALGORITHM_REDUCE_AXIS_HPP__

#include "scalar_algorithm_transformation.hpp"
#include "transformation.hpp"

namespace xios {

class CScalar;
class CAxis;
class CReduceAxisToScalar;
class CReductionAlgorithm;

/*!
  \class CScalarAlgorithmReduceScalar
  Reducing an axis to a scalar
*/
class CScalarAlgorithmReduceScalar : public CScalarAlgorithmTransformation
{
public:
  CScalarAlgorithmReduceScalar(CScalar* scalarDestination, CAxis* axisSource, CReduceAxisToScalar* algo);

  virtual void apply(const std::vector<std::pair<int,double> >& localIndex,
                     const double* dataInput,
                     CArray<double,1>& dataOut,
                     std::vector<bool>& flagInitial,
                     const double& defaultValue);

  virtual void updateData(CArray<double,1>& dataOut);
  
  virtual ~CScalarAlgorithmReduceScalar();

  static bool registerTrans();
protected:
  void computeIndexSourceMapping_(const std::vector<CArray<double,1>* >& dataAuxInputs);

protected:
  CReductionAlgorithm* reduction_;

private:

  static CGenericAlgorithmTransformation* create(CGrid* gridDst, CGrid* gridSrc,
                                                CTransformation<CScalar>* transformation,
                                                int elementPositionInGrid,
                                                std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                std::map<int, int>& elementPositionInGridDst2DomainPosition);
};

}
#endif // __XIOS_SCALAR_ALGORITHM_REDUCE_AXIS_HPP__
