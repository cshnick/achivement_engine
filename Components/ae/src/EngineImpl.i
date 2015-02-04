 // pair.i - SWIG interface
 %module PyAeCore 
 %{
 #include "EngineImpl.h"
 #include <string>
 %}

 // Parse the original header file

 %include "std_string.i"
 %include "std_map.i"
 %include "std_list.i"
 
 %include "Engine.h"
 %include "EngineImpl.h"
 
 // Instantiate some templates
 %template(map_sv) std::map<std::string, AE::variant>;
 %template(achievements_params) std::list<std::map<std::string, AE::variant> >;