#ifndef __TYPE_UTIL_HPP__
#define __TYPE_UTIL_HPP__

#include <string>
namespace xios
{
    class CDomain;
    class CDomainGroup;
    class CField;
    class CFieldGroup;
    class CGrid;
    class CGridGroup;
    class CAxis;
    class CAxisGroup;
    class CFile;
    class CFileGroup;
    class CContext;
    class CContextGroup;
    class CCalendarWrapper;
    class CVariable;
    class CVariableGroup;
    class CInverseAxis;
    class CInverseAxisGroup;
    class CZoomAxis;
    class CZoomAxisGroup;
    class CInterpolateAxis;
    class CInterpolateAxisGroup;
    class CZoomDomain;
    class CZoomDomainGroup;
    class CInterpolateFromFileDomain;
    class CInterpolateFromFileDomainGroup;
    class CGenerateRectilinearDomain;
    class CGenerateRectilinearDomainGroup;

  template <typename T> inline string getStrType(void);

#define macro(T) template <> inline string getStrType<T>(void) { return std::string(#T); }

  macro(short)
  macro(unsigned short)
  macro(int)
  macro(unsigned int)
  macro(long)
  macro(unsigned long)
  macro(float)
  macro(double)
  macro(long double)
  macro(char)
  macro(unsigned char)
  macro(wchar_t)
  macro(bool)
#undef macro

#define macro(T) template <> inline string getStrType<T>(void) { return std::string(#T); }
  macro(CDomain)
  macro(CDomainGroup)
  macro(CField)
  macro(CFieldGroup)
  macro(CGrid)
  macro(CGridGroup)
  macro(CAxis)
  macro(CAxisGroup)
  macro(CFile)
  macro(CFileGroup)
  macro(CContext)
  macro(CContextGroup)
  macro(CCalendarWrapper)
  macro(CVariable)
  macro(CVariableGroup)
  macro(CInverseAxis)
  macro(CInverseAxisGroup)
  macro(CZoomAxis)
  macro(CZoomAxisGroup)
  macro(CInterpolateAxis)
  macro(CInterpolateAxisGroup)
  macro(CZoomDomain)
  macro(CZoomDomainGroup)
  macro(CInterpolateFromFileDomain)
  macro(CInterpolateFromFileDomainGroup)
  macro(CGenerateRectilinearDomain)
  macro(CGenerateRectilinearDomainGroup)
#undef macro
}


#endif
