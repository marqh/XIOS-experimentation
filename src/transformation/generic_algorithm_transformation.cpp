/*!
   \file generic_algorithm_transformation.hpp
   \author Ha NGUYEN
   \since 14 May 2015
   \date 21 Mars 2016

   \brief Interface for all transformation algorithms.
 */
#include "generic_algorithm_transformation.hpp"

namespace xios {

CGenericAlgorithmTransformation::CGenericAlgorithmTransformation()
 : transformationMapping_(), transformationWeight_(), transformationPosition_(), idAuxInputs_()
{
}

/*!
  This function computes the global indexes of grid source, which the grid destination is in demand.
  \param[in] elementPositionInGrid position of an element in a grid .E.g: if grid is composed of domain and axis (in order),
                then position of axis in grid is 2 (since a domain is considered to contain 2 elements (axis)
  \param[in] gridDestGlobalDim global size of each dimension of grid source (all dimension must have the same size except of the one on which transformation is performed)
  \param[in] gridSrcGlobalDim dimension size of source grid (it should share the same size for all dimension, maybe except the domain on which transformation is performed)
  \param[in] globalIndexGridDestSendToServer global index of grid destination on the current client to send to server
  \param[in] localIndexGridSendToServer local index of grid destination on the current client to send to server
  \param[in/out] globaIndexWeightFromDestToSource mapping between transformed global index of grid destination
             and the weighted value as well as global index from grid index source
*/
void CGenericAlgorithmTransformation::computeGlobalSourceIndex(int elementPositionInGrid,
                                                             const std::vector<int>& gridDestGlobalDim,
                                                             const std::vector<int>& gridSrcGlobalDim,
                                                             const std::vector<size_t>& globalIndexGridDestSendToServer,
                                                             const std::vector<int>& localIndexGridSendToServer,
                                                             DestinationIndexMap& globaIndexWeightFromDestToSource)
{
  bool isTransPosEmpty = transformationPosition_.empty();
  boost::unordered_map<size_t,int> globalLocalIndexDestSendToServerMap;
  size_t nbGlobalIndexDest = globalIndexGridDestSendToServer.size();
  for (size_t idx = 0; idx < nbGlobalIndexDest; ++idx)
  {
    globalLocalIndexDestSendToServerMap[globalIndexGridDestSendToServer[idx]] = localIndexGridSendToServer[idx];
  }

  for (size_t idxTrans = 0; idxTrans < transformationMapping_.size(); ++idxTrans)
  {
    std::map<int, std::vector<int> >::const_iterator itbTransMap = transformationMapping_[idxTrans].begin(), itTransMap,
                                                     iteTransMap = transformationMapping_[idxTrans].end();
    std::map<int, std::vector<double> >::const_iterator itTransWeight = transformationWeight_[idxTrans].begin();

    // If transformation position exists
    std::map<int, std::vector<int> >::const_iterator itTransPos, iteTransPos;
    if (!isTransPosEmpty)
    {
      itTransPos  = transformationPosition_[idxTrans].begin(),
      iteTransPos = transformationPosition_[idxTrans].end();
    }
    std::vector<int> emptyTransPos;

    std::vector<std::vector<size_t> > globalIndexSrcGrid;
    std::vector<std::pair<size_t,int> > globalLocalIndexDest;
    for (itTransMap = itbTransMap; itTransMap != iteTransMap; ++itTransMap, ++itTransWeight)
    {
      boost::unordered_map<size_t,int> globalLocalIndexDestMap;
      if (!isTransPosEmpty)
      {
        this->computeGlobalGridIndexFromGlobalIndexElement(itTransMap->first,
                                                           itTransMap->second,
                                                           itTransPos->second,
                                                           elementPositionInGrid,
                                                           gridDestGlobalDim,
                                                           gridSrcGlobalDim,
                                                           globalLocalIndexDestSendToServerMap,
                                                           globalLocalIndexDest,
                                                           globalIndexSrcGrid);
        ++itTransPos;
      }
      else
      {
        this->computeGlobalGridIndexFromGlobalIndexElement(itTransMap->first,
                                                           itTransMap->second,
                                                           emptyTransPos,
                                                           elementPositionInGrid,
                                                           gridDestGlobalDim,
                                                           gridSrcGlobalDim,
                                                           globalLocalIndexDestSendToServerMap,
                                                           globalLocalIndexDest,
                                                           globalIndexSrcGrid);
      }
      std::vector<std::pair<size_t,int> >::const_iterator it = globalLocalIndexDest.begin(), ite = globalLocalIndexDest.end();
      const std::vector<double>& currentVecWeight = itTransWeight->second;

      for (size_t idx = 0; it != ite; ++it, ++idx)
      {
        size_t srcGridSize = globalIndexSrcGrid[idx].size();
        globaIndexWeightFromDestToSource[(it->first)].reserve(srcGridSize);
        for (int i = 0; i < srcGridSize; ++i)
        {
          globaIndexWeightFromDestToSource[(it->first)].push_back(make_pair(it->second, make_pair(globalIndexSrcGrid[idx][i], currentVecWeight[i])));
        }
      }
    }
  }
}

void CGenericAlgorithmTransformation::computeIndexSourceMapping(const std::vector<CArray<double,1>* >& dataAuxInputs)
{
  computeIndexSourceMapping_(dataAuxInputs);
}

std::vector<StdString> CGenericAlgorithmTransformation::getIdAuxInputs()
{
  return idAuxInputs_;
}

}
