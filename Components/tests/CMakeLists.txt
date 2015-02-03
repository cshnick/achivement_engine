project(main_test)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql REQUIRED)
include (${QT_USE_FILE})

include_directories(  
  .
  ${AE_SRC}
  /usr/include/python
)
add_definitions (
  ${DEF_COMMON}
)

set (HEADERS
)
set (SOURCES
  main.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_COMMON}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${C_COMMON}")

set (EXT_LIBS
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}/Calcs
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${BIN_PATH}/Calcs
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})