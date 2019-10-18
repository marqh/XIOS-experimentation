#ifndef __XIOS_CObject__
#define __XIOS_CObject__

/// XIOS headers ///
#include "xios_spl.hpp"

namespace xios
{
  /// ////////////////////// Déclarations ////////////////////// ///

  class CObject
  {
    public:
      /// Destructeur ///
      virtual ~CObject(void);

      /// Accesseurs ///
      const StdString& getId(void) const;
      virtual const StdString& getIdServer() ;
      virtual const StdString& getIdServer(int nsrvPool) ;

      virtual StdString dumpClassAttributes(void);

      /// Mutateurs ///
      void resetId(void);
      void setId(const StdString& id, bool idAutoGenerated = false);

      /// Tests ///
      bool hasId(void) const;
      bool hasAutoGeneratedId(void) const;

      /// Opérateurs ///
      // bool operator==(const CObject& other) const;
      // bool operator!=(const CObject& other) const;

      /// Flux ///
      friend StdOStream& operator<<(StdOStream& os, const CObject& object);

      /// Autres ///
      virtual StdString toString(void) const = 0;
      virtual void fromString(const StdString& str) = 0;

    protected:
      /// Constructeurs ///
      CObject(void);
      explicit CObject(const StdString& id, bool idAutoGenerated = false);
      CObject(const CObject& object);
      CObject(const CObject* const object); // Not implemented.

    private:
      /// Propriétés ///
      StdString id;   // identifiant de l'Object
      bool idDefined; // true si l'object est identifié, false sinon.
      bool idAutoGenerated; //!< true if and only the id was automatically generated
  }; // class CObject
} // namespace xios

#endif // __XIOS_CObject__
