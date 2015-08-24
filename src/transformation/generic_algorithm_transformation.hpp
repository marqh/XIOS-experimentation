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

namespace xios {
  /*!
  \class CGenericAlgorithmTransformation
  This class defines the interface for all other inherted algorithms class
  */
class CGenericAlgorithmTransformation
{
public:
  CGenericAlgorithmTransformation();

  virtual ~CGenericAlgorithmTransformation() {}

  void computeGlobalSourceIndex(int elementPositionInGrid,
                                const std::vector<int>& gridDestGlobalDim,
                                const std::vector<int>& gridSrcGlobalDim,
                                const std::vector<size_t>& globalIndexGridDestSendToServer,
                                std::map<size_t, std::vector<std::pair<size_t,double> > >& globaIndexWeightFromDestToSource);

  /*!
  Compute global index mapping from one element of destination grid to the corresponding element of source grid
  */
  virtual void computeIndexSourceMapping() = 0;

protected:
  /*!
  Compute an array of global index from a global index on an element
    \param[in] destGlobalIndex global index on an element of destination grid
    \param[in] srcGlobalIndex global index(es) on an element of source grid (which are needed by one index on element destination)
    \param[in] elementPositionInGrid position of the element in the grid (for example: a grid with one domain and one axis, position of domain is 1, position of axis is 2)
    \param[in] gridDestGlobalDim dimension size of destination grid (it should share the same size for all dimension, maybe except the element on which transformation is performed)
    \param[in] globalIndexGridDestSendToServer global index of destination grid which are to be sent to server(s), this array is already acsending sorted
    \param[in/out] globalIndexDestGrid array of global index (for 2d grid, this array maybe a line, for 3d, this array may represent a plan). It should be preallocated
    \param[in/out] globalIndexSrcGrid array of global index of source grid (for 2d grid, this array is a line, for 3d, this array represents a plan). It should be preallocated
  */
  virtual void computeGlobalGridIndexFromGlobalIndexElement(int destGlobalIndex,
                                                        const std::vector<int>& srcGlobalIndex,
                                                        int elementPositionInGrid,
                                                        const std::vector<int>& gridDestGlobalDim,
                                                        const std::vector<int>& gridSrcGlobalDim,
                                                        const std::vector<size_t>& globalIndexGridDestSendToServer,
                                                        CArray<size_t,1>& globalIndexDestGrid,
                                                        std::vector<std::vector<size_t> >& globalIndexSrcGrid) = 0;



protected:
  std::map<int, std::vector<int> > transformationMapping_;
  std::map<int, std::vector<double> > transformationWeight_;
};

}
#endif // __XIOS_GENERIC_ALGORITHM_TRANSFORMATION_HPP__
