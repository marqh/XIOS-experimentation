/*!
   \file domain_algorithm_reorder.cpp
   \brief Algorithm for reorder a domain.
 */
#include "domain_algorithm_reorder.hpp"
#include "reorder_domain.hpp"
#include "domain.hpp"
#include "grid.hpp"
#include "grid_transformation_factory_impl.hpp"

namespace xios {
CGenericAlgorithmTransformation* CDomainAlgorithmReorder::create(bool isSource, CGrid* gridDst, CGrid* gridSrc,
                                                             CTransformation<CDomain>* transformation,
                                                             int elementPositionInGrid,
                                                             std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                             std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                             std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                             std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                             std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                             std::map<int, int>& elementPositionInGridDst2DomainPosition)
TRY
{
  std::vector<CDomain*> domainListDestP = gridDst->getDomains();
  std::vector<CDomain*> domainListSrcP  = gridSrc->getDomains();

  CReorderDomain* reorderDomain = dynamic_cast<CReorderDomain*> (transformation);
  int domainDstIndex = elementPositionInGridDst2DomainPosition[elementPositionInGrid];
  int domainSrcIndex = elementPositionInGridSrc2DomainPosition[elementPositionInGrid];

  return (new CDomainAlgorithmReorder(isSource, domainListDestP[domainDstIndex], domainListSrcP[domainSrcIndex], reorderDomain));
}
CATCH

bool CDomainAlgorithmReorder::dummyRegistered_ = CDomainAlgorithmReorder::registerTrans();
bool CDomainAlgorithmReorder::registerTrans()
TRY
{
  return CGridTransformationFactory<CDomain>::registerTransformation(TRANS_REORDER_DOMAIN, create);
}
CATCH

CDomainAlgorithmReorder::CDomainAlgorithmReorder(bool isSource, CDomain* domainDestination, CDomain* domainSource, CReorderDomain* reorderDomain)
: CAlgorithmTransformationNoDataModification(isSource)
TRY
{
  // Input data for checkAttributes()
  // checkDomain
  domainDestination->type.setValue( CDomain::type_attr::rectilinear );
  // Keep a 2D point of view for this transformation
  domainDestination->ni_glo = domainSource->ni_glo;
  domainDestination->nj_glo = domainSource->nj_glo;
  //domainDestination->ni = domainSource->ni;         // Will be computed by checkAttributes
  //domainDestination->nj = domainSource->nj;         //   in function of :
  //domainDestination->ibegin = domainSource->ibegin;>ibegin; //     - domainDestination->i_index
  //domainDestination->jbegin = domainSource->jbegin;>jbegin; //     - domainDestination->j_index
  CArray<size_t,1> sourceGlobalIdx = domainSource->getLocalElement()->getGlobalIndex();
  int indexSize = sourceGlobalIdx.numElements();
  domainDestination->i_index.resize( indexSize );
  domainDestination->j_index.resize( indexSize );
  for (size_t i = 0; i < indexSize ; ++i) {
    domainDestination->i_index(i) = sourceGlobalIdx(i)%domainSource->ni_glo;
    domainDestination->j_index(i) = sourceGlobalIdx(i)/domainSource->ni_glo;
  }
  // else
  //   - domainDestination->ni_glo = domainSource->ni_glo * domainSource->nj_glo;
  //   - domainDestination->nj_glo = 1;
  //   - domainDestination->i_index = sourceGlobalIdx;
  //   - domainDestination->i_index = 0;

  CArray<int,1> sourceWorkflowIdx = domainSource->getLocalView(CElementView::WORKFLOW)->getIndex();
  CArray<int,1> sourceFullIdx     = domainSource->getLocalView(CElementView::FULL    )->getIndex();

  // checkMask -> define domainMask
  domainDestination->mask_1d.resize( indexSize );
  //domainDestination->mask_2d.resize( domainSource->mask_2d.numElements() );
  //domainDestination->mask_2d = domainSource->mask_2d;
  
  // checkDomainData
  domainDestination->data_dim = 1;//domainSource->data_dim;
  //domainDestination->data_ni = domainSource->data_ni;
  //domainDestination->data_nj = domainSource->data_nj;
  //domainDestination->data_ibegin = domainSource->data_ibegin;
  //domainDestination->data_jbegin = domainSource->data_ibegin;
  // checkCompression
  domainDestination->data_i_index.resize( indexSize );
  domainDestination->data_j_index.resize( indexSize );
  domainDestination->data_j_index = 0;
  int countMasked(0);
  for (size_t i = 0; i < indexSize ; ++i) {
    if ( sourceFullIdx(i)==sourceWorkflowIdx(i-countMasked) ) {
      domainDestination->mask_1d(i) = 1;
      domainDestination->data_i_index(i) = sourceFullIdx(i);
      //domainDestination->data_i_index(i) = sourceFullIdx(i)%domainSource->ni -  domainSource->data_ibegin;
      //domainDestination->data_j_index(i) = sourceFullIdx(i)/domainSource->ni -  domainSource->data_jbegin;
    }
    else {
      domainDestination->mask_1d(i) = 0;
      domainDestination->data_i_index(i) = -1;
      //domainDestination->data_j_index(i) = -1;
      countMasked++;
    }
  }

  
  // checkLonLat -> define (bounds_)lon/latvalue
  domainDestination->latvalue_1d.resize( domainSource->latvalue_1d.numElements() );
  domainDestination->lonvalue_1d.resize( domainSource->lonvalue_1d.numElements() );
  domainDestination->latvalue_1d = domainSource->latvalue_1d;
  domainDestination->lonvalue_1d = domainSource->lonvalue_1d;
  domainDestination->latvalue_2d.resize( domainSource->latvalue_2d.numElements() );
  domainDestination->lonvalue_2d.resize( domainSource->lonvalue_2d.numElements() );
  domainDestination->latvalue_2d = domainSource->latvalue_2d;
  domainDestination->lonvalue_2d = domainSource->lonvalue_2d;
  // checkBounds
  domainDestination->bounds_lon_1d.resize( domainSource->bounds_lon_1d.numElements() );
  domainDestination->bounds_lat_1d.resize( domainSource->bounds_lat_1d.numElements() );
  domainDestination->bounds_lon_1d = domainSource->bounds_lon_1d;
  domainDestination->bounds_lat_1d = domainSource->bounds_lat_1d;
  domainDestination->bounds_lon_2d.resize( domainSource->bounds_lon_2d.numElements() );
  domainDestination->bounds_lat_2d.resize( domainSource->bounds_lat_2d.numElements() );
  domainDestination->bounds_lon_2d = domainSource->bounds_lon_2d;
  domainDestination->bounds_lat_2d = domainSource->bounds_lat_2d;
  // checkArea

  reorderDomain->checkValid(domainSource);
  // domainDestination->checkAttributes() will be operated at the end of the transformation definition to define correctly domainDestination views
  //domainDestination->checkAttributes() ; // for now but maybe use domainSource as template for domain destination

  if (domainSource->type !=  CDomain::type_attr::rectilinear)
  {
      ERROR("CDomainAlgorithmReorder::CDomainAlgorithmReorder(CDomain* domainDestination, CDomain* domainSource, CReorderDomain* reorderDomain)",
           << "Domain destination is not rectilinear. This filter work only for rectilinear domain and destination domain with < id = "
           <<domainDestination->getId() <<" > is of type "<<domainDestination->type<<std::endl);
  }
  
  if (domainDestination == domainSource)
  {
    ERROR("CDomainAlgorithmReorder::CDomainAlgorithmReorder(CDomain* domainDestination, CDomain* domainSource, CReorderDomain* reorderDomain)",
           << "Domain source and domain destination are the same. Please make sure domain destination refers to domain source" << std::endl
           << "Domain source " <<domainSource->getId() << std::endl
           << "Domain destination " <<domainDestination->getId() << std::endl);
  }
  
  if (!reorderDomain->invert_lat.isEmpty() && reorderDomain->invert_lat.getValue() )
  {
    CArray<int,1>& j_index=domainDestination->j_index ;
    int nglo = j_index.numElements() ;
    int nj_glo =domainDestination->nj_glo ;
    for (size_t i = 0; i < nglo ; ++i)
    {
      j_index(i)=(nj_glo-1)-j_index(i) ;
    }
  }

  if (!reorderDomain->shift_lon_fraction.isEmpty())
  {
    int ni_glo =domainDestination->ni_glo ;
    int  offset = ni_glo*reorderDomain->shift_lon_fraction ;
    CArray<int,1>& i_index=domainDestination->i_index ;
    int nglo = i_index.numElements() ;
    for (size_t i = 0; i < nglo ; ++i)
    {
      i_index(i)=  (i_index(i)+offset+ni_glo)%ni_glo ;
    }
  }

  if (!reorderDomain->min_lon.isEmpty() && !reorderDomain->max_lon.isEmpty())
  {
    double min_lon=reorderDomain->min_lon ;
    double max_lon=reorderDomain->max_lon ;
    double delta=max_lon-min_lon ;
    
    if (!domainDestination->lonvalue_1d.isEmpty() )
    {
      CArray<double,1>& lon=domainDestination->lonvalue_1d ;
      for (int i=0;i<lon.numElements();++i)
      {
        while  (lon(i) > max_lon) lon(i)=lon(i)-delta ;
        while  (lon(i) < min_lon) lon(i)=lon(i)+delta ;
      }
    }

    if (!domainDestination->bounds_lon_1d.isEmpty() )
    {
      CArray<double,2>& bounds_lon=domainDestination->bounds_lon_1d ;
      for (int i=0;i<bounds_lon.extent(0);++i)
      {
        while  (bounds_lon(0,i) > max_lon) bounds_lon(0,i)=bounds_lon(0,i)-delta ;
        while  (bounds_lon(1,i) > max_lon) bounds_lon(1,i)=bounds_lon(1,i)-delta ;

        while  (bounds_lon(0,i) < min_lon) bounds_lon(0,i)=bounds_lon(0,i)+delta ;
        while  (bounds_lon(1,i) < min_lon) bounds_lon(1,i)=bounds_lon(1,i)+delta ;
      }
    }
  }

  domainDestination->checkAttributes() ;
}
CATCH


}
