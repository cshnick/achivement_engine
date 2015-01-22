project(var_calcs)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})

include_directories(  
.
../src
/usr/include
)
add_definitions (
	-DENABLE_TESTS
)

set (HEADERS
)
set (SOURCES
  Delegates.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=internal" -g -O0)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fvisibility=internal" -g -O0)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=internal")
SET(CMAKE_CXX_FLAGS "-std=c++11")

set (EXT_LIBS
   # curl
   # tidy
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})