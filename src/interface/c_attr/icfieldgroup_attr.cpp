/* ************************************************************************** *
 *               Interface auto generated - do not modify                     *
 * ************************************************************************** */

#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>
#include "xmlioserver.hpp"
#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"
#include "icutil.hpp"
#include "icdate.hpp"
#include "timer.hpp"
#include "node_type.hpp"

extern "C"
{
  typedef xios::CFieldGroup* fieldgroup_Ptr;

  void cxios_set_fieldgroup_add_offset(fieldgroup_Ptr fieldgroup_hdl, double add_offset)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->add_offset.setValue(add_offset);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_add_offset(fieldgroup_Ptr fieldgroup_hdl, double* add_offset)
  {
    CTimer::get("XIOS").resume();
    *add_offset = fieldgroup_hdl->add_offset.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_add_offset(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->add_offset.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_axis_ref(fieldgroup_Ptr fieldgroup_hdl, const char * axis_ref, int axis_ref_size)
  {
    std::string axis_ref_str;
    if (!cstr2string(axis_ref, axis_ref_size, axis_ref_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->axis_ref.setValue(axis_ref_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_axis_ref(fieldgroup_Ptr fieldgroup_hdl, char * axis_ref, int axis_ref_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->axis_ref.getInheritedValue(), axis_ref, axis_ref_size))
      ERROR("void cxios_get_fieldgroup_axis_ref(fieldgroup_Ptr fieldgroup_hdl, char * axis_ref, int axis_ref_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_axis_ref(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->axis_ref.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_default_value(fieldgroup_Ptr fieldgroup_hdl, double default_value)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->default_value.setValue(default_value);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_default_value(fieldgroup_Ptr fieldgroup_hdl, double* default_value)
  {
    CTimer::get("XIOS").resume();
    *default_value = fieldgroup_hdl->default_value.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_default_value(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->default_value.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_detect_missing_value(fieldgroup_Ptr fieldgroup_hdl, bool detect_missing_value)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->detect_missing_value.setValue(detect_missing_value);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_detect_missing_value(fieldgroup_Ptr fieldgroup_hdl, bool* detect_missing_value)
  {
    CTimer::get("XIOS").resume();
    *detect_missing_value = fieldgroup_hdl->detect_missing_value.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_detect_missing_value(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->detect_missing_value.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_domain_ref(fieldgroup_Ptr fieldgroup_hdl, const char * domain_ref, int domain_ref_size)
  {
    std::string domain_ref_str;
    if (!cstr2string(domain_ref, domain_ref_size, domain_ref_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->domain_ref.setValue(domain_ref_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_domain_ref(fieldgroup_Ptr fieldgroup_hdl, char * domain_ref, int domain_ref_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->domain_ref.getInheritedValue(), domain_ref, domain_ref_size))
      ERROR("void cxios_get_fieldgroup_domain_ref(fieldgroup_Ptr fieldgroup_hdl, char * domain_ref, int domain_ref_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_domain_ref(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->domain_ref.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_enabled(fieldgroup_Ptr fieldgroup_hdl, bool enabled)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->enabled.setValue(enabled);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_enabled(fieldgroup_Ptr fieldgroup_hdl, bool* enabled)
  {
    CTimer::get("XIOS").resume();
    *enabled = fieldgroup_hdl->enabled.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_enabled(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->enabled.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_field_ref(fieldgroup_Ptr fieldgroup_hdl, const char * field_ref, int field_ref_size)
  {
    std::string field_ref_str;
    if (!cstr2string(field_ref, field_ref_size, field_ref_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->field_ref.setValue(field_ref_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_field_ref(fieldgroup_Ptr fieldgroup_hdl, char * field_ref, int field_ref_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->field_ref.getInheritedValue(), field_ref, field_ref_size))
      ERROR("void cxios_get_fieldgroup_field_ref(fieldgroup_Ptr fieldgroup_hdl, char * field_ref, int field_ref_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_field_ref(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->field_ref.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_freq_offset(fieldgroup_Ptr fieldgroup_hdl, cxios_duration freq_offset_c)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->freq_offset.allocate();
    CDuration& freq_offset = fieldgroup_hdl->freq_offset.get();
    freq_offset.year = freq_offset_c.year;
    freq_offset.month = freq_offset_c.month;
    freq_offset.day = freq_offset_c.day;
    freq_offset.hour = freq_offset_c.hour;
    freq_offset.minute = freq_offset_c.minute;
    freq_offset.second = freq_offset_c.second;
    freq_offset.timestep = freq_offset_c.timestep;
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_freq_offset(fieldgroup_Ptr fieldgroup_hdl, cxios_duration* freq_offset_c)
  {
    CTimer::get("XIOS").resume();
    CDuration freq_offset = fieldgroup_hdl->freq_offset.getInheritedValue();
    freq_offset_c->year = freq_offset.year;
    freq_offset_c->month = freq_offset.month;
    freq_offset_c->day = freq_offset.day;
    freq_offset_c->hour = freq_offset.hour;
    freq_offset_c->minute = freq_offset.minute;
    freq_offset_c->second = freq_offset.second;
    freq_offset_c->timestep = freq_offset.timestep;
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_freq_offset(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->freq_offset.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_freq_op(fieldgroup_Ptr fieldgroup_hdl, cxios_duration freq_op_c)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->freq_op.allocate();
    CDuration& freq_op = fieldgroup_hdl->freq_op.get();
    freq_op.year = freq_op_c.year;
    freq_op.month = freq_op_c.month;
    freq_op.day = freq_op_c.day;
    freq_op.hour = freq_op_c.hour;
    freq_op.minute = freq_op_c.minute;
    freq_op.second = freq_op_c.second;
    freq_op.timestep = freq_op_c.timestep;
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_freq_op(fieldgroup_Ptr fieldgroup_hdl, cxios_duration* freq_op_c)
  {
    CTimer::get("XIOS").resume();
    CDuration freq_op = fieldgroup_hdl->freq_op.getInheritedValue();
    freq_op_c->year = freq_op.year;
    freq_op_c->month = freq_op.month;
    freq_op_c->day = freq_op.day;
    freq_op_c->hour = freq_op.hour;
    freq_op_c->minute = freq_op.minute;
    freq_op_c->second = freq_op.second;
    freq_op_c->timestep = freq_op.timestep;
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_freq_op(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->freq_op.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_grid_ref(fieldgroup_Ptr fieldgroup_hdl, const char * grid_ref, int grid_ref_size)
  {
    std::string grid_ref_str;
    if (!cstr2string(grid_ref, grid_ref_size, grid_ref_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->grid_ref.setValue(grid_ref_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_grid_ref(fieldgroup_Ptr fieldgroup_hdl, char * grid_ref, int grid_ref_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->grid_ref.getInheritedValue(), grid_ref, grid_ref_size))
      ERROR("void cxios_get_fieldgroup_grid_ref(fieldgroup_Ptr fieldgroup_hdl, char * grid_ref, int grid_ref_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_grid_ref(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->grid_ref.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_group_ref(fieldgroup_Ptr fieldgroup_hdl, const char * group_ref, int group_ref_size)
  {
    std::string group_ref_str;
    if (!cstr2string(group_ref, group_ref_size, group_ref_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->group_ref.setValue(group_ref_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_group_ref(fieldgroup_Ptr fieldgroup_hdl, char * group_ref, int group_ref_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->group_ref.getInheritedValue(), group_ref, group_ref_size))
      ERROR("void cxios_get_fieldgroup_group_ref(fieldgroup_Ptr fieldgroup_hdl, char * group_ref, int group_ref_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_group_ref(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->group_ref.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_level(fieldgroup_Ptr fieldgroup_hdl, int level)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->level.setValue(level);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_level(fieldgroup_Ptr fieldgroup_hdl, int* level)
  {
    CTimer::get("XIOS").resume();
    *level = fieldgroup_hdl->level.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_level(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->level.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_long_name(fieldgroup_Ptr fieldgroup_hdl, const char * long_name, int long_name_size)
  {
    std::string long_name_str;
    if (!cstr2string(long_name, long_name_size, long_name_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->long_name.setValue(long_name_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_long_name(fieldgroup_Ptr fieldgroup_hdl, char * long_name, int long_name_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->long_name.getInheritedValue(), long_name, long_name_size))
      ERROR("void cxios_get_fieldgroup_long_name(fieldgroup_Ptr fieldgroup_hdl, char * long_name, int long_name_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_long_name(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->long_name.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_name(fieldgroup_Ptr fieldgroup_hdl, const char * name, int name_size)
  {
    std::string name_str;
    if (!cstr2string(name, name_size, name_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->name.setValue(name_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_name(fieldgroup_Ptr fieldgroup_hdl, char * name, int name_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->name.getInheritedValue(), name, name_size))
      ERROR("void cxios_get_fieldgroup_name(fieldgroup_Ptr fieldgroup_hdl, char * name, int name_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_name(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->name.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_operation(fieldgroup_Ptr fieldgroup_hdl, const char * operation, int operation_size)
  {
    std::string operation_str;
    if (!cstr2string(operation, operation_size, operation_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->operation.setValue(operation_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_operation(fieldgroup_Ptr fieldgroup_hdl, char * operation, int operation_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->operation.getInheritedValue(), operation, operation_size))
      ERROR("void cxios_get_fieldgroup_operation(fieldgroup_Ptr fieldgroup_hdl, char * operation, int operation_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_operation(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->operation.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_prec(fieldgroup_Ptr fieldgroup_hdl, int prec)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->prec.setValue(prec);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_prec(fieldgroup_Ptr fieldgroup_hdl, int* prec)
  {
    CTimer::get("XIOS").resume();
    *prec = fieldgroup_hdl->prec.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_prec(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->prec.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_scale_factor(fieldgroup_Ptr fieldgroup_hdl, double scale_factor)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->scale_factor.setValue(scale_factor);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_scale_factor(fieldgroup_Ptr fieldgroup_hdl, double* scale_factor)
  {
    CTimer::get("XIOS").resume();
    *scale_factor = fieldgroup_hdl->scale_factor.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_scale_factor(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->scale_factor.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_standard_name(fieldgroup_Ptr fieldgroup_hdl, const char * standard_name, int standard_name_size)
  {
    std::string standard_name_str;
    if (!cstr2string(standard_name, standard_name_size, standard_name_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->standard_name.setValue(standard_name_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_standard_name(fieldgroup_Ptr fieldgroup_hdl, char * standard_name, int standard_name_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->standard_name.getInheritedValue(), standard_name, standard_name_size))
      ERROR("void cxios_get_fieldgroup_standard_name(fieldgroup_Ptr fieldgroup_hdl, char * standard_name, int standard_name_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_standard_name(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->standard_name.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_unit(fieldgroup_Ptr fieldgroup_hdl, const char * unit, int unit_size)
  {
    std::string unit_str;
    if (!cstr2string(unit, unit_size, unit_str)) return;
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->unit.setValue(unit_str);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_unit(fieldgroup_Ptr fieldgroup_hdl, char * unit, int unit_size)
  {
    CTimer::get("XIOS").resume();
    if (!string_copy(fieldgroup_hdl->unit.getInheritedValue(), unit, unit_size))
      ERROR("void cxios_get_fieldgroup_unit(fieldgroup_Ptr fieldgroup_hdl, char * unit, int unit_size)", << "Input string is too short");
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_unit(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->unit.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_valid_max(fieldgroup_Ptr fieldgroup_hdl, double valid_max)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->valid_max.setValue(valid_max);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_valid_max(fieldgroup_Ptr fieldgroup_hdl, double* valid_max)
  {
    CTimer::get("XIOS").resume();
    *valid_max = fieldgroup_hdl->valid_max.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_valid_max(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->valid_max.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }


  void cxios_set_fieldgroup_valid_min(fieldgroup_Ptr fieldgroup_hdl, double valid_min)
  {
    CTimer::get("XIOS").resume();
    fieldgroup_hdl->valid_min.setValue(valid_min);
    CTimer::get("XIOS").suspend();
  }

  void cxios_get_fieldgroup_valid_min(fieldgroup_Ptr fieldgroup_hdl, double* valid_min)
  {
    CTimer::get("XIOS").resume();
    *valid_min = fieldgroup_hdl->valid_min.getInheritedValue();
    CTimer::get("XIOS").suspend();
  }

  bool cxios_is_defined_fieldgroup_valid_min(fieldgroup_Ptr fieldgroup_hdl)
  {
     CTimer::get("XIOS").resume();
     bool isDefined = fieldgroup_hdl->valid_min.hasInheritedValue();
     CTimer::get("XIOS").suspend();
     return isDefined;
  }
}
