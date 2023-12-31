/*!
   \brief Algorithm for reordering domain.
 */
#ifndef __XIOS_DOMAIN_ALGORITHM_REORDER_HPP__
#define __XIOS_DOMAIN_ALGORITHM_REORDER_HPP__

#include "algorithm_transformation_no_data_modification.hpp"
#include "transformation.hpp"

namespace xios 
{

  class CDomain;
  class CReorderDomain;

  /*!
    \class CDomainAlgorithmReorder
    Reordering data on domain
  */
  class CDomainAlgorithmReorder : public CAlgorithmTransformationNoDataModification  
  {
    public:
      CDomainAlgorithmReorder(bool isSource, CDomain* domainDestination, CDomain* domainSource, CReorderDomain* reorderDomain);

      virtual ~CDomainAlgorithmReorder() {}

      static bool registerTrans();
      virtual StdString getAlgoName() {return "\\nreorder_domain";}

  
    public:
      static shared_ptr<CGenericAlgorithmTransformation> create(bool isSource, CGrid* gridDst, CGrid* gridSrc,
                                                     CTransformation<CDomain>* transformation,
                                                     int elementPositionInGrid,
                                                     std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                     std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                     std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                     std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                     std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                     std::map<int, int>& elementPositionInGridDst2DomainPosition);
      static bool dummyRegistered_;
};

}
#endif // __XIOS_DOMAIN_ALGORITHM_REORDER_HPP__
