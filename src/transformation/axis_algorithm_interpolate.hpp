/*!
   \file axis_algorithm_interpolate.hpp
   \author Ha NGUYEN
   \since 23 June 2015
   \date 23 June 2015

   \brief Algorithm for interpolation on an axis.
 */
#ifndef __XIOS_AXIS_ALGORITHM_INTERPOLATE_HPP__
#define __XIOS_AXIS_ALGORITHM_INTERPOLATE_HPP__

#include "axis_algorithm_transformation.hpp"
#include "axis.hpp"
#include "interpolate_axis.hpp"

namespace xios {
/*!
  \class CAxisAlgorithmInterpolate
  Implementing interpolation on axis
  The values on axis source are assumed monotonic
*/
class CAxisAlgorithmInterpolate : public CAxisAlgorithmTransformation
{
public:
  CAxisAlgorithmInterpolate(CAxis* axisDestination, CAxis* axisSource, CInterpolateAxis* interpAxis);

  virtual ~CAxisAlgorithmInterpolate() {}

  virtual void computeIndexSourceMapping();

private:
  void retrieveAllAxisValue(std::vector<double>& recvBuff, std::vector<int>& indexVec);
  void computeInterpolantPoint(const std::vector<double>& recvBuff, const std::vector<int>& indexVec);
  void computeWeightedValueAndMapping(const std::map<int, std::vector<std::pair<int,double> > >& interpolatingIndexValues);

private:
  // Interpolation order
  int order_;
};

}
#endif // __XIOS_AXIS_ALGORITHM_INTERPOLATE_HPP__