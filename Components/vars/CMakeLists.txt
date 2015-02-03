project(var_calcs)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})

include_directories(  
  .
  ${AE_SRC}
)
add_definitions (
  ${DEF_COMMON}
)

set (HEADERS
)
set (SOURCES
  Delegates.cpp
  ${AE_SRC}/Conventions.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_COMMON}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${C_COMMON}")

set (EXT_LIBS
)

add_custom_target(cp_to_install
  COMMAND(/usr/bin/echo "tratata")
)

add_dependencies(cp_to_install ${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${PYTHON_BINDINGS_INSTALL_PATH}/Calcs
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/libvar_calcs.so ${PYTHON_BINDINGS_INSTALL_PATH}/Calcs
   DEPENDS(${PROJECT_NAME})
)
#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}/Calcs
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${BIN_PATH}/Calcs
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})