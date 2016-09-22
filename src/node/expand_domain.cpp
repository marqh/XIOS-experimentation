#include "expand_domain.hpp"
#include "type.hpp"

namespace xios {

  /// ////////////////////// Définitions ////////////////////// ///

  CExpandDomain::CExpandDomain(void)
    : CObjectTemplate<CExpandDomain>(), CExpandDomainAttributes(), CTransformation<CDomain>()
  { /* Ne rien faire de plus */ }

  CExpandDomain::CExpandDomain(const StdString & id)
    : CObjectTemplate<CExpandDomain>(id), CExpandDomainAttributes(), CTransformation<CDomain>()
  { /* Ne rien faire de plus */ }

  CExpandDomain::~CExpandDomain(void)
  {}

  CTransformation<CDomain>* CExpandDomain::create(const StdString& id, xml::CXMLNode* node)
  {
    CExpandDomain* expandDomain = CExpandDomainGroup::get("expand_domain_definition")->createChild(id);
    if (node) expandDomain->parse(*node);
    return static_cast<CTransformation<CDomain>*>(expandDomain);
  }

  bool CExpandDomain::_dummyRegistered = CExpandDomain::registerTrans();
  bool CExpandDomain::registerTrans()
  {
    registerTransformation(TRANS_COMPUTE_CONNECTIVITY_DOMAIN, CExpandDomain::create);
  }

  //----------------------------------------------------------------

  StdString CExpandDomain::GetName(void)    { return StdString("expand_domain"); }
  StdString CExpandDomain::GetDefName(void) { return StdString("expand_domain"); }
  ENodeType CExpandDomain::GetType(void)    { return eExpandDomain; }

  void CExpandDomain::checkValid(CDomain* domainDst)
  {
  }

}