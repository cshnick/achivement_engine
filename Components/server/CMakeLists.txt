cmake_minimum_required(VERSION 2.8.11)

project(tinny_http_server)
set(CMAKE_PREFIX_PATH ${QT_5_DIR})

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
  main.cpp
  http_server.cpp
  ${AE_SRC}/Conventions.cpp
  RequestProcessorAeQt.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_COMMON}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${C_COMMON}")

set (EXT_LIBS
  pthread
  event
)

find_package(Qt5 COMPONENTS Quick Core Network Xml Widgets Sql)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_SOURCE_DIR}/test_content ${BIN_PATH}
)

target_link_libraries(${PROJECT_NAME} ${EXT_LIBS} ${AeCore})
qt5_use_modules(${PROJECT_NAME} Sql Quick Core Network Xml Widgets)