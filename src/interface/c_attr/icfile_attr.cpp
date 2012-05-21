/* ************************************************************************** *
 *               Interface auto generated - do not modify                   *
 * ************************************************************************** */

#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>
#include "xmlioserver.hpp"
#include "attribute_template_impl.hpp"
#include "object_template_impl.hpp"
#include "group_template_impl.hpp"
#include "icutil.hpp"
#include "timer.hpp"

extern "C"
{
  typedef xios::CFile*  file_Ptr;
  
  void cxios_set_file_description(file_Ptr file_hdl, const char * description, int description_size)
  {
    std::string description_str;
    if(!cstr2string(description, description_size, description_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->description.setValue(description_str);
    file_hdl->sendAttributToServer(file_hdl->description);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_description(file_Ptr file_hdl, char * description, int description_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->description.getValue(),description , description_size))
      ERROR("void cxios_get_file_description(file_Ptr file_hdl, char * description, int description_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_enabled(file_Ptr file_hdl, bool enabled)
  {
     CTimer::get("XIOS").resume();
    file_hdl->enabled.setValue(enabled);
    file_hdl->sendAttributToServer(file_hdl->enabled);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_enabled(file_Ptr file_hdl, bool* enabled)
  {
    *enabled = file_hdl->enabled.getValue();
  }
  
  
  void cxios_set_file_name(file_Ptr file_hdl, const char * name, int name_size)
  {
    std::string name_str;
    if(!cstr2string(name, name_size, name_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->name.setValue(name_str);
    file_hdl->sendAttributToServer(file_hdl->name);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_name(file_Ptr file_hdl, char * name, int name_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->name.getValue(),name , name_size))
      ERROR("void cxios_get_file_name(file_Ptr file_hdl, char * name, int name_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_name_suffix(file_Ptr file_hdl, const char * name_suffix, int name_suffix_size)
  {
    std::string name_suffix_str;
    if(!cstr2string(name_suffix, name_suffix_size, name_suffix_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->name_suffix.setValue(name_suffix_str);
    file_hdl->sendAttributToServer(file_hdl->name_suffix);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_name_suffix(file_Ptr file_hdl, char * name_suffix, int name_suffix_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->name_suffix.getValue(),name_suffix , name_suffix_size))
      ERROR("void cxios_get_file_name_suffix(file_Ptr file_hdl, char * name_suffix, int name_suffix_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_output_freq(file_Ptr file_hdl, const char * output_freq, int output_freq_size)
  {
    std::string output_freq_str;
    if(!cstr2string(output_freq, output_freq_size, output_freq_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->output_freq.setValue(output_freq_str);
    file_hdl->sendAttributToServer(file_hdl->output_freq);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_output_freq(file_Ptr file_hdl, char * output_freq, int output_freq_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->output_freq.getValue(),output_freq , output_freq_size))
      ERROR("void cxios_get_file_output_freq(file_Ptr file_hdl, char * output_freq, int output_freq_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_output_level(file_Ptr file_hdl, int output_level)
  {
     CTimer::get("XIOS").resume();
    file_hdl->output_level.setValue(output_level);
    file_hdl->sendAttributToServer(file_hdl->output_level);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_output_level(file_Ptr file_hdl, int* output_level)
  {
    *output_level = file_hdl->output_level.getValue();
  }
  
  
  void cxios_set_file_par_access(file_Ptr file_hdl, const char * par_access, int par_access_size)
  {
    std::string par_access_str;
    if(!cstr2string(par_access, par_access_size, par_access_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->par_access.setValue(par_access_str);
    file_hdl->sendAttributToServer(file_hdl->par_access);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_par_access(file_Ptr file_hdl, char * par_access, int par_access_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->par_access.getValue(),par_access , par_access_size))
      ERROR("void cxios_get_file_par_access(file_Ptr file_hdl, char * par_access, int par_access_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_split_freq(file_Ptr file_hdl, const char * split_freq, int split_freq_size)
  {
    std::string split_freq_str;
    if(!cstr2string(split_freq, split_freq_size, split_freq_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->split_freq.setValue(split_freq_str);
    file_hdl->sendAttributToServer(file_hdl->split_freq);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_split_freq(file_Ptr file_hdl, char * split_freq, int split_freq_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->split_freq.getValue(),split_freq , split_freq_size))
      ERROR("void cxios_get_file_split_freq(file_Ptr file_hdl, char * split_freq, int split_freq_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_sync_freq(file_Ptr file_hdl, const char * sync_freq, int sync_freq_size)
  {
    std::string sync_freq_str;
    if(!cstr2string(sync_freq, sync_freq_size, sync_freq_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->sync_freq.setValue(sync_freq_str);
    file_hdl->sendAttributToServer(file_hdl->sync_freq);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_sync_freq(file_Ptr file_hdl, char * sync_freq, int sync_freq_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->sync_freq.getValue(),sync_freq , sync_freq_size))
      ERROR("void cxios_get_file_sync_freq(file_Ptr file_hdl, char * sync_freq, int sync_freq_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  void cxios_set_file_type(file_Ptr file_hdl, const char * type, int type_size)
  {
    std::string type_str;
    if(!cstr2string(type, type_size, type_str)) return;
     CTimer::get("XIOS").resume();
    file_hdl->type.setValue(type_str);
    file_hdl->sendAttributToServer(file_hdl->type);
     CTimer::get("XIOS").suspend();
  }
  
  void cxios_get_file_type(file_Ptr file_hdl, char * type, int type_size)
  {
     CTimer::get("XIOS").resume();
    if(!string_copy(file_hdl->type.getValue(),type , type_size))
      ERROR("void cxios_get_file_type(file_Ptr file_hdl, char * type, int type_size)", <<"Input string is to short");
     CTimer::get("XIOS").suspend();
  }
  
  
  
}
