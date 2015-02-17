project(main_test)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt4 COMPONENTS QtCore QtXml QtSql QtNetwork REQUIRED)
include (${QT_USE_FILE})

SET(db_test db_test)
SET(main_http_test main_http_test)

include_directories(  
  .
  ${AE_SRC}
)
add_definitions (
  ${DEF_COMMON}
  -DENABLE_TESTS
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
add_executable(${db_test} ${db_test}.cpp)
add_executable(${main_http_test} ${main_http_test}.cpp)

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${BIN_PATH}
)
ADD_CUSTOM_COMMAND( 
   TARGET ${db_test}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${db_test} ${BIN_PATH}
)
ADD_CUSTOM_COMMAND( 
   TARGET ${main_http_test}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${main_http_test} ${BIN_PATH}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS} ${AeCore})
target_link_libraries(${db_test} ${QT_LIBRARIES} ${EXT_LIBS} ${AeCore})
target_link_libraries(${main_http_test} ${QT_LIBRARIES} ${EXT_LIBS} ${AeCore})