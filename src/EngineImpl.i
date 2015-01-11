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
 
 %template(pairii) std::pair<int,int>;
 %template(pairdi) std::pair<double,int>;
 %template(pairid) std::pair<int, double>;
 %template(map_ss) std::map<std::string, std::string>;
