#include "extract_domain_to_axis.hpp"
#include "axis_algorithm_extract_domain.hpp"
#include "type.hpp"
#include "axis.hpp"
#include "domain.hpp"

namespace xios {

  /// ////////////////////// Définitions ////////////////////// ///

  CExtractDomainToAxis::CExtractDomainToAxis(void)
    : CObjectTemplate<CExtractDomainToAxis>(), CExtractDomainToAxisAttributes(), CTransformation<CAxis>()
  { /* Ne rien faire de plus */ }

  CExtractDomainToAxis::CExtractDomainToAxis(const StdString & id)
    : CObjectTemplate<CExtractDomainToAxis>(id), CExtractDomainToAxisAttributes(), CTransformation<CAxis>()
  { /* Ne rien faire de plus */ }

  CExtractDomainToAxis::~CExtractDomainToAxis(void)
  {}

  CTransformation<CAxis>* CExtractDomainToAxis::create(const StdString& id, xml::CXMLNode* node)
  {
    CExtractDomainToAxis* extractDomain = CExtractDomainToAxisGroup::get("extract_domain_to_axis_definition")->createChild(id);
    if (node) extractDomain->parse(*node);
    return static_cast<CTransformation<CAxis>*>(extractDomain);
  }

  bool CExtractDomainToAxis::registerTrans()
  {
    return registerTransformation(TRANS_EXTRACT_DOMAIN_TO_AXIS, {create, getTransformation});
  }

  bool CExtractDomainToAxis::_dummyRegistered = CExtractDomainToAxis::registerTrans();

  //----------------------------------------------------------------

  StdString CExtractDomainToAxis::GetName(void)    { return StdString("extract_domain_to_axis"); }
  StdString CExtractDomainToAxis::GetDefName(void) { return StdString("extract_domain_to_axis"); }
  ENodeType CExtractDomainToAxis::GetType(void)    { return eExtractDomainToAxis; }

  void CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)
  {
    if (CDomain::type_attr::unstructured == domainSrc->type)
      ERROR("CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)",
       << "Domain reduction is only supported for rectilinear or curvillinear grid."
       << "Domain source " <<domainSrc->getId() << std::endl
       << "Axis destination " << axisDst->getId());

    int domain_ni_glo = domainSrc->ni_glo;
    int domain_nj_glo = domainSrc->nj_glo;

    if (this->direction.isEmpty())
      ERROR("CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)",
             << "A direction to apply the operation must be defined. It should be: 'iDir' or 'jDir'"
             << "Domain source " <<domainSrc->getId() << std::endl
             << "Axis destination " << axisDst->getId());
 
    if (this->position.isEmpty())
      ERROR("CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)",
             << "Position to extract axis must be defined. " << std::endl
             << "Domain source " <<domainSrc->getId() << std::endl
             << "Axis destination " << axisDst->getId());
    
    switch (direction)
    {
      case direction_attr::jDir:
        if ((position < 0) || (position >= domain_ni_glo))
        ERROR("CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)",
          << "Extract domain along j, position should be inside 0 and ni_glo-1 of domain source"
          << "Domain source " <<domainSrc->getId() << " has ni_glo " << domain_ni_glo << std::endl
          << "Axis destination " << axisDst->getId() << std::endl
          << "Position " << position);
         break;

      case direction_attr::iDir:
        if ((position < 0) || (position >= domain_nj_glo))
        ERROR("CExtractDomainToAxis::checkValid(CAxis* axisDst, CDomain* domainSrc)",
          << "Extract domain along i, position should be inside 0 and nj_glo-1 of domain source"
          << "Domain source " <<domainSrc->getId() << " has nj_glo " << domain_nj_glo << std::endl
          << "Axis destination " << axisDst->getId() << std::endl
          << "Position " << position);
        break;

      default:
        break;
    }
  }

  shared_ptr<CGenericAlgorithmTransformation> CExtractDomainToAxis::createAlgorithm(bool isSource,
                                                        CGrid* gridDst, CGrid* gridSrc,
                                                        int elementPositionInGrid,
                                                        std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                        std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                        std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                        std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                        std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                        std::map<int, int>& elementPositionInGridDst2DomainPosition)
  {
    return CAxisAlgorithmExtractDomain::create(isSource, gridDst, gridSrc, this, elementPositionInGrid, 
                       elementPositionInGridSrc2ScalarPosition, elementPositionInGridSrc2AxisPosition, elementPositionInGridSrc2DomainPosition,
                       elementPositionInGridDst2ScalarPosition, elementPositionInGridDst2AxisPosition, elementPositionInGridDst2DomainPosition);
  }
}
