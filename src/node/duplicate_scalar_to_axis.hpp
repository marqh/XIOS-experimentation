#ifndef __XIOS_CDuplicateScalarToAxis__
#define __XIOS_CDuplicateScalarToAxis__

/// xios headers ///
#include "xios_spl.hpp"
#include "attribute_enum.hpp"
#include "attribute_enum_impl.hpp"
#include "attribute_array.hpp"
#include "declare_attribute.hpp"
#include "object_template.hpp"
#include "group_factory.hpp"
#include "declare_group.hpp"
#include "transformation.hpp"

namespace xios {
  /// ////////////////////// Déclarations ////////////////////// ///
  class CDuplicateScalarToAxisGroup;
  class CDuplicateScalarToAxisAttributes;
  class CDuplicateScalarToAxis;
  class CAxis;
  class CScalar;
  class CGenericAlgorithmTransformation ;
  class CGrid;

  ///--------------------------------------------------------------

  // Declare/Define CFileAttribute
  BEGIN_DECLARE_ATTRIBUTE_MAP(CDuplicateScalarToAxis)
#include "duplicate_scalar_to_axis_attribute.conf"
  END_DECLARE_ATTRIBUTE_MAP(CDuplicateScalarToAxis)

  ///--------------------------------------------------------------
  /*!
    \class CDuplicateScalarToAxis
    This class describes duplicate_scalar in xml file.
  */
  class CDuplicateScalarToAxis
    : public CObjectTemplate<CDuplicateScalarToAxis>
    , public CDuplicateScalarToAxisAttributes
    , public CTransformation<CAxis>
  {
    public :
      typedef CObjectTemplate<CDuplicateScalarToAxis> SuperClass;
      typedef CDuplicateScalarToAxisAttributes SuperClassAttribute;
      typedef CDuplicateScalarToAxis MyClass ;
      typedef CTransformation<CAxis> SuperTransform ;

    public :
      /// Constructeurs ///
      CDuplicateScalarToAxis(void);
      explicit CDuplicateScalarToAxis(const StdString& id);

      /// Destructeur ///
      virtual ~CDuplicateScalarToAxis(void);

      virtual void checkValid(CAxis* axisDst, CScalar* scalarSrc);

      /// Accesseurs statiques ///
      static StdString GetName(void);
      static StdString GetDefName(void);
      static ENodeType GetType(void);
      const string& getId(void) { return this->SuperClass::getId();}
      ETranformationType getTransformationType(void) { return TRANS_DUPLICATE_SCALAR_TO_AXIS ;}
      static CTransformation<CAxis>* getTransformation(const StdString& id) { return SuperClass::get(id);}
      virtual void inheritFrom(SuperTransform* srcTransform) { solveDescInheritance(true, this->SuperClass::get((MyClass*)srcTransform)) ;}
      virtual shared_ptr<CGenericAlgorithmTransformation> createAlgorithm(bool isSource,
                                                               CGrid* gridDst, CGrid* gridSrc,
                                                               int elementPositionInGrid,
                                                               std::map<int, int>& elementPositionInGridSrc2ScalarPosition,
                                                               std::map<int, int>& elementPositionInGridSrc2AxisPosition,
                                                               std::map<int, int>& elementPositionInGridSrc2DomainPosition,
                                                               std::map<int, int>& elementPositionInGridDst2ScalarPosition,
                                                               std::map<int, int>& elementPositionInGridDst2AxisPosition,
                                                               std::map<int, int>& elementPositionInGridDst2DomainPosition)  ;
    private:
      static bool registerTrans();
      static CTransformation<CAxis>* create(const StdString& id, xml::CXMLNode* node);
      static bool _dummyRegistered;
  }; // class CReduceAxisToAxis

  DECLARE_GROUP(CDuplicateScalarToAxis);
} // namespace xios

#endif // ___XIOS_CDuplicateScalarToAxis__
