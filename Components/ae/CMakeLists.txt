project(AeCore)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})

include_directories(  
  .
  src
  /usr/include/python
  src/ExpressionParser
)
add_definitions (
  ${DEF_COMMON}
)

set (HEADERS
)
set (SOURCES
  src/EngineImpl.cpp
  src/ExpressionParser/node.cpp
  src/ExpressionParser/ExpressionParser.cpp
  src/Conventions.cpp
)
set (PYTHON_SRC
  src/EngineImpl_wrap.cxx
)

set (CMAKE_BUILD_EXECUTABLE FALSE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_COMMON}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${C_COMMON}")

set (EXT_LIBS
   dl
)

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl_wrap.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/AeCore.py
   COMMAND /usr/bin/swig -c++ -python ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.i ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.h ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine.h
)

if (${CMAKE_BUILD_EXECUTABLE}) 
	add_executable(${PROJECT_NAME} ${SOURCES} ${SRC_LIST} ${HEADERS})
else (${CMAKE_BUILD_EXECUTABLE})
	add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PYTHON_SRC} ${SRC_LIST} ${HEADERS})
endif (${CMAKE_BUILD_EXECUTABLE}) 

if (NOT ${CMAKE_BUILD_EXECUTABLE})
	ADD_CUSTOM_COMMAND( 
	   TARGET ${PROJECT_NAME}
	   POST_BUILD
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/libAeCore.so ${CMAKE_CURRENT_BINARY_DIR}/_AeCore.so
	   COMMAND /usr/bin/cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/src/AeCore.py ${CMAKE_CURRENT_BINARY_DIR}
	   COMMAND /usr/bin/mkdir -p ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/libAeCore.so ${PYTHON_BINDINGS_INSTALL_PATH}/_AeCore.so
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/AeCore.py ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/touch ${PYTHON_BINDINGS_INSTALL_PATH}/__init__.py
)
endif (NOT ${CMAKE_BUILD_EXECUTABLE})
#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${BIN_PATH}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})


