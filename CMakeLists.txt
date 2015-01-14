project(AeCore)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})


#aux_source_directory(. SRC_LIST)
include_directories(  
.
src
/usr/include/python
/usr/include
)

set (HEADERS
)
set (SOURCES
  src/main.cpp
  src/EngineImpl.cpp
)
set (PYTHON_SRC
  src/EngineImpl_wrap.cxx
)

set (CMAKE_BUILD_EXECUTABLE FALSE)
set (CMAKE_BUILD_LIBRARY TRUE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden" -g -O0)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden" -g -O0)
SET(CMAKE_CXX_FLAGS "-std=c++11")

set (EXT_LIBS
   # curl
   # tidy
)

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl_wrap.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/AeCore.py
   COMMAND /usr/bin/swig -c++ -python ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.i ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.h ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine.h
)

if (${CMAKE_BUILD_EXECUTABLE}) 
	add_executable(${PROJECT_NAME} ${SOURCES} ${SRC_LIST} ${HEADERS})
elseif (${CMAKE_BUILD_LIBRARY})
	add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PYTHON_SRC} ${SRC_LIST} ${HEADERS})
endif (${CMAKE_BUILD_EXECUTABLE}) 

if (${CMAKE_BUILD_LIBRARY})
	ADD_CUSTOM_COMMAND( 
	   TARGET ${PROJECT_NAME}
	   POST_BUILD
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/libAeCore.so ${CMAKE_CURRENT_BINARY_DIR}/_AeCore.so
	   COMMAND /usr/bin/cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/src/AeCore.py ${CMAKE_CURRENT_BINARY_DIR}
	   COMMAND /usr/bin/mkdir -p /home/ilia/Documents/script/python/achivement_engine
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/libAeCore.so /home/ilia/Documents/script/python/achivement_engine/_AeCore.so
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/AeCore.py /home/ilia/Documents/script/python/achivement_engine
	   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.i
)
endif (${CMAKE_BUILD_LIBRARY})

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})


