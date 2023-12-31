! * ************************************************************************** *
! *               Interface auto generated - do not modify                     *
! * ************************************************************************** *
#include "../fortran/xios_fortran_prefix.hpp"

MODULE extract_axis_interface_attr
  USE, INTRINSIC :: ISO_C_BINDING

  INTERFACE
    ! Do not call directly / interface FORTRAN 2003 <-> C99

    SUBROUTINE cxios_set_extract_axis_begin(extract_axis_hdl, begin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
      INTEGER (KIND=C_INT)      , VALUE :: begin
    END SUBROUTINE cxios_set_extract_axis_begin

    SUBROUTINE cxios_get_extract_axis_begin(extract_axis_hdl, begin) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
      INTEGER (KIND=C_INT)             :: begin
    END SUBROUTINE cxios_get_extract_axis_begin

    FUNCTION cxios_is_defined_extract_axis_begin(extract_axis_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_axis_begin
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
    END FUNCTION cxios_is_defined_extract_axis_begin


    SUBROUTINE cxios_set_extract_axis_index(extract_axis_hdl, index, extent) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE       :: extract_axis_hdl
      INTEGER (KIND=C_INT)     , DIMENSION(*) :: index
      INTEGER (kind = C_INT), DIMENSION(*)     :: extent
    END SUBROUTINE cxios_set_extract_axis_index

    SUBROUTINE cxios_get_extract_axis_index(extract_axis_hdl, index, extent) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE       :: extract_axis_hdl
      INTEGER (KIND=C_INT)     , DIMENSION(*) :: index
      INTEGER (kind = C_INT), DIMENSION(*)     :: extent
    END SUBROUTINE cxios_get_extract_axis_index

    FUNCTION cxios_is_defined_extract_axis_index(extract_axis_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_axis_index
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
    END FUNCTION cxios_is_defined_extract_axis_index


    SUBROUTINE cxios_set_extract_axis_n(extract_axis_hdl, n) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
      INTEGER (KIND=C_INT)      , VALUE :: n
    END SUBROUTINE cxios_set_extract_axis_n

    SUBROUTINE cxios_get_extract_axis_n(extract_axis_hdl, n) BIND(C)
      USE ISO_C_BINDING
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
      INTEGER (KIND=C_INT)             :: n
    END SUBROUTINE cxios_get_extract_axis_n

    FUNCTION cxios_is_defined_extract_axis_n(extract_axis_hdl) BIND(C)
      USE ISO_C_BINDING
      LOGICAL(kind=C_BOOL) :: cxios_is_defined_extract_axis_n
      INTEGER (kind = C_INTPTR_T), VALUE :: extract_axis_hdl
    END FUNCTION cxios_is_defined_extract_axis_n

  END INTERFACE

END MODULE extract_axis_interface_attr
