project(AeCore)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})

set(PYTHON_AE_WRAPPER_NAME PyAeWrapper)
set(PY_MODULE_NAME PyAeCore)

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


add_library(${PROJECT_NAME} SHARED ${SOURCES}  ${SRC_LIST} ${HEADERS})
if (PYTHON_WRAPPER)
	add_library(${PYTHON_AE_WRAPPER_NAME} SHARED ${PYTHON_SRC} ${HEADERS})
endif (PYTHON_WRAPPER)

IF (PYTHON_WRAPPER)
	ADD_CUSTOM_COMMAND( 
	   TARGET ${PYTHON_AE_WRAPPER_NAME}
	   POST_BUILD
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PYTHON_AE_WRAPPER_NAME}.so ${CMAKE_CURRENT_BINARY_DIR}/_${PY_MODULE_NAME}.so
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_SOURCE_DIR}/src/${PY_MODULE_NAME}.py ${CMAKE_CURRENT_BINARY_DIR}
	   COMMAND /usr/bin/mkdir -p ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/_${PY_MODULE_NAME}.so ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PY_MODULE_NAME}.py ${PYTHON_BINDINGS_INSTALL_PATH}
	   COMMAND /usr/bin/touch ${PYTHON_BINDINGS_INSTALL_PATH}/__init__.py
	)
endif (PYTHON_WRAPPER)

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${BIN_PATH}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})
if (PYTHON_WRAPPER)
	target_link_libraries(${PYTHON_AE_WRAPPER_NAME} ${EXT_LIBS} ${PROJECT_NAME})
endif (PYTHON_WRAPPER)

