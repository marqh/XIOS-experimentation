! * ************************************************************************** *
! *               Interface auto generated - do not modify                     *
! * ************************************************************************** *
#include "../fortran/xios_fortran_prefix.hpp"

MODULE extract_domain_interface_attr
  USE, INTRINSIC :: ISO_C_BINDING

  INTERFACE
    ! Do not call directly / interface FORTRAN 2003 <-> C99

    SUBROUTINE cxios_set_extract_domain_ibegin(extract_domain_hdl, ibegin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)      , VALUE :: ibegin
    END SUBROUTINE cxios_set_extract_domain_ibegin

    SUBROUTINE cxios_get_extract_domain_ibegin(extract_domain_hdl, ibegin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)             :: ibegin
    END SUBROUTINE cxios_get_extract_domain_ibegin

    FUNCTION cxios_is_defined_extract_domain_ibegin(extract_domain_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_domain_ibegin
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
    END FUNCTION cxios_is_defined_extract_domain_ibegin


    SUBROUTINE cxios_set_extract_domain_jbegin(extract_domain_hdl, jbegin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)      , VALUE :: jbegin
    END SUBROUTINE cxios_set_extract_domain_jbegin

    SUBROUTINE cxios_get_extract_domain_jbegin(extract_domain_hdl, jbegin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)             :: jbegin
    END SUBROUTINE cxios_get_extract_domain_jbegin

    FUNCTION cxios_is_defined_extract_domain_jbegin(extract_domain_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_domain_jbegin
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
    END FUNCTION cxios_is_defined_extract_domain_jbegin


    SUBROUTINE cxios_set_extract_domain_ni(extract_domain_hdl, ni) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)      , VALUE :: ni
    END SUBROUTINE cxios_set_extract_domain_ni

    SUBROUTINE cxios_get_extract_domain_ni(extract_domain_hdl, ni) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)             :: ni
    END SUBROUTINE cxios_get_extract_domain_ni

    FUNCTION cxios_is_defined_extract_domain_ni(extract_domain_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_domain_ni
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
    END FUNCTION cxios_is_defined_extract_domain_ni


    SUBROUTINE cxios_set_extract_domain_nj(extract_domain_hdl, nj) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)      , VALUE :: nj
    END SUBROUTINE cxios_set_extract_domain_nj

    SUBROUTINE cxios_get_extract_domain_nj(extract_domain_hdl, nj) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
      INTEGER (KIND=C_INT)             :: nj
    END SUBROUTINE cxios_get_extract_domain_nj

    FUNCTION cxios_is_defined_extract_domain_nj(extract_domain_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_domain_nj
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_domain_hdl
    END FUNCTION cxios_is_defined_extract_domain_nj

  END INTERFACE

END MODULE extract_domain_interface_attr
