 // pair.i - SWIG interface
 %module AeCore 
 %{
 #include "EngineImpl.h"
 #include <string>
 %}

 // Parse the original header file

 %include "std_string.i"
 %include "std_map.i"
 
 %include "Engine.h"
 %include "EngineImpl.h"
 
 // Instantiate some templates
 %template(map_sv) std::map<std::string, AE::variant>;