/*!
   \file axis_algorithm_interpolate.cpp
   \author Ha NGUYEN
   \since 23 June 2015
   \date 02 Jul 2015

   \brief Algorithm for interpolation on an axis.
 */
#include "axis_algorithm_interpolate.hpp"
#include "axis.hpp"
#include "interpolate_axis.hpp"
#include <algorithm>
#include "context.hpp"
#include "context_client.hpp"
#include "utils.hpp"
#include "grid.hpp"
#include "grid_transformation_factory_impl.hpp"
#include "distribution_client.hpp"
#include "timer.hpp"

namespace xios {
shared_ptr<CGenericAlgorithmTransformation> CAxisAlgorithmInterpolate::create(bool isSource, CGrid* gridDst, CGrid* gridSrc,
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

  CInterpolateAxis* interpolateAxis = dynamic_cast<CInterpolateAxis*> (transformation);
  int axisDstIndex = elementPositionInGridDst2AxisPosition[elementPositionInGrid];
  int axisSrcIndex = elementPositionInGridSrc2AxisPosition[elementPositionInGrid];

  return make_shared<CAxisAlgorithmInterpolate>(isSource, axisListDestP[axisDstIndex], axisListSrcP[axisSrcIndex], interpolateAxis);
}
CATCH

bool CAxisAlgorithmInterpolate::dummyRegistered_ = CAxisAlgorithmInterpolate::registerTrans();
bool CAxisAlgorithmInterpolate::registerTrans()
TRY
{
  /// descativate for now
  // return CGridTransformationFactory<CAxis>::registerTransformation(TRANS_INTERPOLATE_AXIS, create);
  return false;
}
CATCH

CAxisAlgorithmInterpolate::CAxisAlgorithmInterpolate(bool isSource, CAxis* axisDestination, CAxis* axisSource, CInterpolateAxis* interpAxis)
: CAlgorithmTransformationWeight(isSource), coordinate_(), transPosition_(), axisSrc_(axisSource), axisDest_(axisDestination)
TRY
{
  interpAxis->checkValid(axisSource);
  axisDestination->checkAttributes() ;

  order_ = interpAxis->order.getValue();
  if (!interpAxis->coordinate.isEmpty())
  {
    coordinate_ = interpAxis->coordinate.getValue();
//    this->idAuxInputs_.resize(1);
//    this->idAuxInputs_[0] = coordinate_;
  }
  std::vector<CArray<double,1>* > dataAuxInputs ;
  computeRemap(dataAuxInputs) ;
  this->computeAlgorithm(axisSource->getLocalView(CElementView::WORKFLOW), axisDestination->getLocalView(CElementView::WORKFLOW), false, false) ;
}
CATCH

/*!
  Compute the index mapping between axis on grid source and one on grid destination
*/
void CAxisAlgorithmInterpolate::computeRemap(const std::vector<CArray<double,1>* >& dataAuxInputs)
TRY
{
  CTimer::get("CAxisAlgorithmInterpolate::computeIndexSourceMapping_").resume() ;
  CContext* context = CContext::getCurrent();
  int nbClient = context->intraCommSize_;
  CArray<bool,1>& axisMask = axisSrc_->mask;
  int srcSize  = axisSrc_->n_glo.getValue();
  std::vector<CArray<double,1> > vecAxisValue;

  // Fill in axis value from coordinate
  fillInAxisValue(vecAxisValue, dataAuxInputs);
  std::vector<double> valueSrc(srcSize);
  std::vector<double> recvBuff(srcSize);
  std::vector<int> indexVec(srcSize);

  for (int idx = 0; idx < vecAxisValue.size(); ++idx)
  {
    CArray<double,1>& axisValue = vecAxisValue[idx];
    retrieveAllAxisValue(axisValue, axisMask, recvBuff, indexVec);
    XIOSAlgorithms::sortWithIndex<double, CVectorStorage>(recvBuff, indexVec);
    for (int i = 0; i < srcSize; ++i) valueSrc[i] = recvBuff[indexVec[i]];
    computeInterpolantPoint(valueSrc, indexVec, idx);
  }
  CTimer::get("CAxisAlgorithmInterpolate::computeIndexSourceMapping_").suspend() ;
}
CATCH

/*!
  Compute the interpolant points
  Assume that we have all value of axis source, with these values, need to calculate weight (coeff) of Lagrange polynomial
  \param [in] axisValue all value of axis source
  \param [in] tranPos position of axis on a domain
*/
void CAxisAlgorithmInterpolate::computeInterpolantPoint(const std::vector<double>& axisValue,
                                                        const std::vector<int>& indexVec,
                                                        int transPos)
TRY
{
  std::vector<double>::const_iterator itb = axisValue.begin(), ite = axisValue.end();
  std::vector<double>::const_iterator itLowerBound, itUpperBound, it, iteRange, itfirst, itsecond;
  const double sfmax = NumTraits<double>::sfmax();
  const double precision = NumTraits<double>::dummy_precision();

  int ibegin = axisDest_->begin.getValue();
  CArray<double,1>& axisDestValue = axisDest_->value;
  int numValue = axisDestValue.numElements();
  std::map<int, std::vector<std::pair<int,double> > > interpolatingIndexValues;

  for (int idx = 0; idx < numValue; ++idx)
  {
    bool outOfRange = false;
    double destValue = axisDestValue(idx);
    if (destValue < *itb) outOfRange = true;

    itLowerBound = std::lower_bound(itb, ite, destValue);
    itUpperBound = std::upper_bound(itb, ite, destValue);
    if ((ite != itUpperBound) && (sfmax == *itUpperBound)) itUpperBound = ite;

    if ((ite == itLowerBound) || (ite == itUpperBound)) outOfRange = true;

    // We don't do extrapolation FOR NOW, maybe in the future
    if (!outOfRange)
    {
      if ((itLowerBound == itUpperBound) && (itb != itLowerBound)) --itLowerBound;
      double distanceToLower = destValue - *itLowerBound;
      double distanceToUpper = *itUpperBound - destValue;
      int order = (order_ + 1) - 2;
      bool down = (distanceToLower < distanceToUpper) ? true : false;
      for (int k = 0; k < order; ++k)
      {
        if ((itb != itLowerBound) && down)
        {
          --itLowerBound;
          distanceToLower = destValue - *itLowerBound;
          down = (distanceToLower < distanceToUpper) ? true : false;
          continue;
        }
        if ((ite != itUpperBound) && (sfmax != *itUpperBound))
        {
          ++itUpperBound;
          distanceToUpper = *itUpperBound - destValue;
          down = (distanceToLower < distanceToUpper) ? true : false;

        }
      }

      iteRange = (ite == itUpperBound) ? itUpperBound : itUpperBound + 1;
      itsecond = it = itLowerBound; ++itsecond;
      while (it < iteRange)
      {
        while ( (itsecond < ite) && ((*itsecond -*it) < precision) )
        { ++itsecond; ++it; }
        int index = std::distance(itb, it);
        interpolatingIndexValues[idx+ibegin].push_back(make_pair(indexVec[index],*it));
        ++it; ++itsecond;
      }

    }
  }
  computeWeightedValueAndMapping(interpolatingIndexValues, transPos);
}
CATCH

/*!
  Compute weight (coeff) of Lagrange's polynomial
  \param [in] interpolatingIndexValues the necessary axis value to calculate the coeffs
*/
void CAxisAlgorithmInterpolate::computeWeightedValueAndMapping(const std::map<int, std::vector<std::pair<int,double> > >& interpolatingIndexValues, int transPos)
TRY
{
  TransformationIndexMap& transMap = this->transformationMapping_;
  TransformationWeightMap& transWeight = this->transformationWeight_;
  std::map<int, std::vector<std::pair<int,double> > >::const_iterator itb = interpolatingIndexValues.begin(), it,
                                                                      ite = interpolatingIndexValues.end();
  int ibegin = axisDest_->begin.getValue();
  for (it = itb; it != ite; ++it)
  {
    int globalIndexDest = it->first;
    double localValue = axisDest_->value(globalIndexDest - ibegin);
    const std::vector<std::pair<int,double> >& interpVal = it->second;
    int interpSize = interpVal.size();
    transMap[globalIndexDest].resize(interpSize);
    transWeight[globalIndexDest].resize(interpSize);
    for (int idx = 0; idx < interpSize; ++idx)
    {
      int index = interpVal[idx].first;
      double weight = 1.0;

      for (int k = 0; k < interpSize; ++k)
      {
        if (k == idx) continue;
        weight *= (localValue - interpVal[k].second);
        weight /= (interpVal[idx].second - interpVal[k].second);
      }
      transMap[globalIndexDest][idx] = index;
      transWeight[globalIndexDest][idx] = weight;
/*
      if (!transPosition_.empty())
      {
        (this->transformationPosition_[transPos])[globalIndexDest] = transPosition_[transPos];
      }
*/
    }
  }
/*
  if (!transPosition_.empty() && this->transformationPosition_[transPos].empty())
    (this->transformationPosition_[transPos])[0] = transPosition_[transPos];
*/
}
CATCH

/*!
  Each client retrieves all values of an axis
  \param [in/out] recvBuff buffer for receiving values (already allocated)
  \param [in/out] indexVec mapping between values and global index of axis
*/
void CAxisAlgorithmInterpolate::retrieveAllAxisValue(const CArray<double,1>& axisValue, const CArray<bool,1>& axisMask,
                                                     std::vector<double>& recvBuff, std::vector<int>& indexVec)
TRY
{
  CContext* context = CContext::getCurrent();
  int nbClient = context->intraCommSize_;

  int srcSize  = axisSrc_->n_glo.getValue();
  int numValue = axisValue.numElements();

  if (srcSize == numValue)  // Only one client or axis not distributed
  {
    for (int idx = 0; idx < srcSize; ++idx)
    {
      if (axisMask(idx))
      {
        recvBuff[idx] = axisValue(idx);
        indexVec[idx] = idx;
      }
      else
      {
        recvBuff[idx] = NumTraits<double>::sfmax();
        indexVec[idx] = -1;
      }
    }

  }
  else // Axis distributed
  {
    double* sendValueBuff = new double [numValue];
    int* sendIndexBuff = new int [numValue];
    int* recvIndexBuff = new int [srcSize];

    int ibegin = axisSrc_->begin.getValue();
    for (int idx = 0; idx < numValue; ++idx)
    {
      if (axisMask(idx))
      {
        sendValueBuff[idx] = axisValue(idx);
        sendIndexBuff[idx] = idx + ibegin;
      }
      else
      {
        sendValueBuff[idx] = NumTraits<double>::sfmax();
        sendIndexBuff[idx] = -1;
      }
    }

    int* recvCount=new int[nbClient];
    MPI_Allgather(&numValue,1,MPI_INT,recvCount,1,MPI_INT,context->intraComm_);

    int* displ=new int[nbClient];
    displ[0]=0 ;
    for(int n=1;n<nbClient;n++) displ[n]=displ[n-1]+recvCount[n-1];

    // Each client have enough global info of axis
    MPI_Allgatherv(sendIndexBuff,numValue,MPI_INT,recvIndexBuff,recvCount,displ,MPI_INT,context->intraComm_);
    MPI_Allgatherv(sendValueBuff,numValue,MPI_DOUBLE,&(recvBuff[0]),recvCount,displ,MPI_DOUBLE,context->intraComm_);

    for (int idx = 0; idx < srcSize; ++idx)
    {
      indexVec[idx] = recvIndexBuff[idx];
    }

    delete [] displ;
    delete [] recvCount;
    delete [] recvIndexBuff;
    delete [] sendIndexBuff;
    delete [] sendValueBuff;
  }
}
CATCH

/*!
  Fill in axis value dynamically from a field whose grid is composed of a domain and an axis
  \param [in/out] vecAxisValue vector axis value filled in from input field
*/
void CAxisAlgorithmInterpolate::fillInAxisValue(std::vector<CArray<double,1> >& vecAxisValue,
                                                const std::vector<CArray<double,1>* >& dataAuxInputs)
TRY
{
  if (coordinate_.empty())
  {
    vecAxisValue.resize(1);
    vecAxisValue[0].resize(axisSrc_->value.numElements());
    vecAxisValue[0] = axisSrc_->value;
//    this->transformationMapping_.resize(1);
//    this->transformationWeight_.resize(1);
  }
  else
  {
/*
    CField* field = CField::get(coordinate_);
    CGrid* grid = field->getGrid();

    std::vector<CDomain*> domListP = grid->getDomains();
    std::vector<CAxis*> axisListP = grid->getAxis();
    if (domListP.empty() || axisListP.empty() || (1 < domListP.size()) || (1 < axisListP.size()))
      ERROR("CAxisAlgorithmInterpolate::fillInAxisValue(std::vector<CArray<double,1> >& vecAxisValue)",
             << "XIOS only supports dynamic interpolation with coordinate (field) associated with grid composed of a domain and an axis"
             << "Coordinate (field) id = " <<field->getId() << std::endl
             << "Associated grid id = " << grid->getId());

    CDomain* dom = domListP[0];
    size_t vecAxisValueSize = dom->i_index.numElements();
    size_t vecAxisValueSizeWithMask = 0;
    for (size_t idx = 0; idx < vecAxisValueSize; ++idx)
    {
      if (dom->domainMask(idx)) ++vecAxisValueSizeWithMask;
    }

    int niGlobDom = dom->ni_glo.getValue();
    vecAxisValue.resize(vecAxisValueSizeWithMask);
    if (transPosition_.empty())
    {
      size_t indexMask = 0;
      transPosition_.resize(vecAxisValueSizeWithMask);
      for (size_t idx = 0; idx < vecAxisValueSize; ++idx)
      {
        if (dom->domainMask(idx))
        {
          transPosition_[indexMask].resize(1);
          transPosition_[indexMask][0] = (dom->i_index)(idx) + niGlobDom * (dom->j_index)(idx);
          ++indexMask;
        }

      }
    }
    this->transformationMapping_.resize(vecAxisValueSizeWithMask);
    this->transformationWeight_.resize(vecAxisValueSizeWithMask);
    this->transformationPosition_.resize(vecAxisValueSizeWithMask);

    const CDistributionClient::GlobalLocalDataMap& globalLocalIndexSendToServer = grid->getClientDistribution()->getGlobalLocalDataSendToServer();
    CDistributionClient::GlobalLocalDataMap::const_iterator itIndex, iteIndex = globalLocalIndexSendToServer.end();
    size_t axisSrcSize = axisSrc_->index.numElements();
    std::vector<int> globalDimension = grid->getGlobalDimension();

    size_t indexMask = 0;
    for (size_t idx = 0; idx < vecAxisValueSize; ++idx)
    {
      if (dom->domainMask(idx))
      {
        size_t axisValueSize = 0;
        for (size_t jdx = 0; jdx < axisSrcSize; ++jdx)
        {
          size_t globalIndex = ((dom->i_index)(idx) + (dom->j_index)(idx)*globalDimension[0]) + (axisSrc_->index)(jdx)*globalDimension[0]*globalDimension[1];
          if (iteIndex != globalLocalIndexSendToServer.find(globalIndex))
          {
            ++axisValueSize;
          }
        }

        vecAxisValue[indexMask].resize(axisValueSize);
        axisValueSize = 0;
        for (size_t jdx = 0; jdx < axisSrcSize; ++jdx)
        {
          size_t globalIndex = ((dom->i_index)(idx) + (dom->j_index)(idx)*globalDimension[0]) + (axisSrc_->index)(jdx)*globalDimension[0]*globalDimension[1];
          itIndex = globalLocalIndexSendToServer.find(globalIndex);
          if (iteIndex != itIndex)
          {
            vecAxisValue[indexMask](axisValueSize) = (*dataAuxInputs[0])(itIndex->second);
            ++axisValueSize;
          }
        }
        ++indexMask;
      }
    }
  */
  }
}
CATCH

}
