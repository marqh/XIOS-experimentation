/* ************************************************************************** *
 *               Interface auto generated - do not modify                   *
 * ************************************************************************** */

#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>
#include "xmlioserver.hpp"
#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"
#include "icutil.hpp"
#include "timer.hpp"
#include "node_type.hpp"

extern "C"
{
  typedef xios::CGrid*  grid_Ptr;
  
  void cxios_set_grid_axis_ref(grid_Ptr grid_hdl, const char * axis_ref, int axis_ref_size)
  {
    std::string axis_ref_str;
    if(!cstr2string(axis_ref, axis_ref_size, axis_ref_str)) return;
     CTimer::get("XIOS").resume();
    grid_hdl->axis_ref.setValue(axis_ref_str);
    grid_hdl->sendAttributToServer(grid_hdl->axis_ref);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_grid_axis_ref(grid_Ptr grid_hdl, char * axis_ref, int axis_ref_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(grid_hdl->axis_ref.getValue(),axis_ref , axis_ref_size))
      ERROR("void cxios_get_grid_axis_ref(grid_Ptr grid_hdl, char * axis_ref, int axis_ref_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_grid_description(grid_Ptr grid_hdl, const char * description, int description_size)
  {
    std::string description_str;
    if(!cstr2string(description, description_size, description_str)) return;
     CTimer::get("XIOS").resume();
    grid_hdl->description.setValue(description_str);
    grid_hdl->sendAttributToServer(grid_hdl->description);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_grid_description(grid_Ptr grid_hdl, char * description, int description_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(grid_hdl->description.getValue(),description , description_size))
      ERROR("void cxios_get_grid_description(grid_Ptr grid_hdl, char * description, int description_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_grid_domain_ref(grid_Ptr grid_hdl, const char * domain_ref, int domain_ref_size)
  {
    std::string domain_ref_str;
    if(!cstr2string(domain_ref, domain_ref_size, domain_ref_str)) return;
     CTimer::get("XIOS").resume();
    grid_hdl->domain_ref.setValue(domain_ref_str);
    grid_hdl->sendAttributToServer(grid_hdl->domain_ref);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_grid_domain_ref(grid_Ptr grid_hdl, char * domain_ref, int domain_ref_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(grid_hdl->domain_ref.getValue(),domain_ref , domain_ref_size))
      ERROR("void cxios_get_grid_domain_ref(grid_Ptr grid_hdl, char * domain_ref, int domain_ref_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_grid_name(grid_Ptr grid_hdl, const char * name, int name_size)
  {
    std::string name_str;
    if(!cstr2string(name, name_size, name_str)) return;
     CTimer::get("XIOS").resume();
    grid_hdl->name.setValue(name_str);
    grid_hdl->sendAttributToServer(grid_hdl->name);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_grid_name(grid_Ptr grid_hdl, char * name, int name_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(grid_hdl->name.getValue(),name , name_size))
      ERROR("void cxios_get_grid_name(grid_Ptr grid_hdl, char * name, int name_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  
}
