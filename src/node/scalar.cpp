#include "scalar.hpp"

#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"
#include "object_factory.hpp"
#include "xios_spl.hpp"
#include "type.hpp"

namespace xios {

   /// ////////////////////// Définitions ////////////////////// ///

   CScalar::CScalar(void)
      : CObjectTemplate<CScalar>()
      , CScalarAttributes()
      , relFiles()
   { /* Ne rien faire de plus */ }

   CScalar::CScalar(const StdString & id)
      : CObjectTemplate<CScalar>(id)
      , CScalarAttributes()
      , relFiles()
   { /* Ne rien faire de plus */ }

   CScalar::~CScalar(void)
   { /* Ne rien faire de plus */ }

   std::map<StdString, ETranformationType> CScalar::transformationMapList_ = std::map<StdString, ETranformationType>();
   bool CScalar::dummyTransformationMapList_ = CScalar::initializeTransformationMap(CScalar::transformationMapList_);
   bool CScalar::initializeTransformationMap(std::map<StdString, ETranformationType>& m)
   {
     m["reduce_axis"]   = TRANS_REDUCE_AXIS_TO_SCALAR;
     m["extract_axis"]  = TRANS_EXTRACT_AXIS_TO_SCALAR;
     m["reduce_domain"] = TRANS_REDUCE_DOMAIN_TO_SCALAR;
     m["reduce_scalar"] = TRANS_REDUCE_SCALAR_TO_SCALAR;
   }

   StdString CScalar::GetName(void)   { return (StdString("scalar")); }
   StdString CScalar::GetDefName(void){ return (CScalar::GetName()); }
   ENodeType CScalar::GetType(void)   { return (eScalar); }

   CScalar* CScalar::createScalar()
   {
     CScalar* scalar = CScalarGroup::get("scalar_definition")->createChild();
     return scalar;
   }

   bool CScalar::IsWritten(const StdString & filename) const
   {
      return (this->relFiles.find(filename) != this->relFiles.end());
   }

   void CScalar::addRelFile(const StdString& filename)
   {
      this->relFiles.insert(filename);
   }

   void CScalar::checkAttributes(void)
   {
      if (checkAttributes_done_) return ;

      checkAttributes_done_ = true ; 
   }

  void CScalar::checkAttributesOnClient()
  {

  }

  /*!
  \brief Check if a scalar is completed
  Before make any scalar processing, we must be sure that all scalar informations have
  been sent, for exemple when reading a grid in a file or when grid elements are sent by an
  other context (coupling). So all direct reference of the scalar (scalar_ref) must be also completed
  \return true if scalar and scalar reference are completed
  */
  bool CScalar::checkIfCompleted(void)
  {
    if (hasDirectScalarReference()) if (!getDirectScalarReference()->checkIfCompleted()) return false;
    return isCompleted_ ;
  }

  /*!
  \brief Set a scalar as completed
   When all information about a scalar have been received, the scalar is tagged as completed and is
   suitable for processing
  */
  void CScalar::setCompleted(void)
  {
    if (hasDirectScalarReference()) getDirectScalarReference()->setCompleted() ;
    isCompleted_=true ;
  }

  /*!
  \brief Set a scalar as uncompleted
   When informations about a scalar are expected from a grid reading from file or coupling, the scalar is 
   tagged as uncompleted and is not suitable for processing
  */
  void CScalar::setUncompleted(void)
  {
    if (hasDirectScalarReference()) getDirectScalarReference()->setUncompleted() ;
    isCompleted_=false ;
  }




  /*!
    Compare two scalar objects. 
    They are equal if only if they have identical attributes as well as their values.
    Moreover, they must have the same transformations.
  \param [in] scalar Compared scalar
  \return result of the comparison
  */
  bool CScalar::isEqual(CScalar* obj)
  {
    vector<StdString> excludedAttr;
    excludedAttr.push_back("scalar_ref");
    bool objEqual = SuperClass::isEqual(obj, excludedAttr);
    if (!objEqual) return objEqual;

    TransMapTypes thisTrans = this->getAllTransformations();
    TransMapTypes objTrans  = obj->getAllTransformations();

    TransMapTypes::const_iterator it, itb, ite;
    std::vector<ETranformationType> thisTransType, objTransType;
    for (it = thisTrans.begin(); it != thisTrans.end(); ++it)
      thisTransType.push_back(it->first);
    for (it = objTrans.begin(); it != objTrans.end(); ++it)
      objTransType.push_back(it->first);

    if (thisTransType.size() != objTransType.size()) return false;
    for (int idx = 0; idx < thisTransType.size(); ++idx)
      objEqual &= (thisTransType[idx] == objTransType[idx]);

    return objEqual;
  }

  CTransformation<CScalar>* CScalar::addTransformation(ETranformationType transType, const StdString& id)
  {
    transformationMap_.push_back(std::make_pair(transType, CTransformation<CScalar>::createTransformation(transType,id)));
    return transformationMap_.back().second;
  }

  bool CScalar::hasTransformation()
  {
    return (!transformationMap_.empty());
  }

  void CScalar::setTransformations(const TransMapTypes& scalarTrans)
  {
    transformationMap_ = scalarTrans;
  }

  CScalar::TransMapTypes CScalar::getAllTransformations(void)
  {
    return transformationMap_;
  }

  void CScalar::duplicateTransformation(CScalar* src)
  {
    if (src->hasTransformation())
    {
      this->setTransformations(src->getAllTransformations());
    }
  }

  /*!
   * Go through the hierarchy to find the scalar from which the transformations must be inherited
   */
  void CScalar::solveInheritanceTransformation()
  {
    if (hasTransformation() || !hasDirectScalarReference())
      return;

    CScalar* scalar = this;
    std::vector<CScalar*> refScalar;
    while (!scalar->hasTransformation() && scalar->hasDirectScalarReference())
    {
      refScalar.push_back(scalar);
      scalar = scalar->getDirectScalarReference();
    }

    if (scalar->hasTransformation())
      for (size_t i = 0; i < refScalar.size(); ++i)
        refScalar[i]->setTransformations(scalar->getAllTransformations());
  }

  void CScalar::sendScalarToFileServer(CContextClient* client)
  {
    if (sendScalarToFileServer_done_.count(client)!=0) return ;
    else sendScalarToFileServer_done_.insert(client) ;

    StdString scalarDefRoot("scalar_definition");
    CScalarGroup* scalarPtr = CScalarGroup::get(scalarDefRoot);
    this->sendAllAttributesToServer(client);
  }
  /*!
    Parse children nodes of a scalar in xml file.
    \param node child node to process
  */
  void CScalar::parse(xml::CXMLNode & node)
  {
    SuperClass::parse(node);

    if (node.goToChildElement())
    {
      StdString nodeElementName;
      do
      {
        StdString nodeId("");
        if (node.getAttributes().end() != node.getAttributes().find("id"))
        { nodeId = node.getAttributes()["id"]; }

        nodeElementName = node.getElementName();
        std::map<StdString, ETranformationType>::const_iterator ite = transformationMapList_.end(), it;
        it = transformationMapList_.find(nodeElementName);
        if (ite != it)
        {
          transformationMap_.push_back(std::make_pair(it->second, CTransformation<CScalar>::createTransformation(it->second,
                                                                                                                 nodeId,
                                                                                                                 &node)));
        }
        else
        {
          ERROR("void CScalar::parse(xml::CXMLNode & node)",
                << "The transformation " << nodeElementName << " has not been supported yet.");
        }
      } while (node.goToNextElement()) ;
      node.goToParentElement();
    }
  }

  // Definition of some macros
  DEFINE_REF_FUNC(Scalar,scalar)

} // namespace xios
