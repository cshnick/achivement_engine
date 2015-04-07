cmake_minimum_required(VERSION 2.8.11)

project(sqlWrapper)
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
  ./src/Select.cpp
  ./src/Update.cpp
  ./src/InsertInto.cpp
  ./src/CreateTable.cpp
  ./src/AlterTable.cpp
  ${AE_SRC}/Conventions.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CXX_COMMON}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${C_COMMON}")

set (EXT_LIBS
)

find_package(Qt5 COMPONENTS Quick Core Network Xml Widgets Sql)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so ${BIN_PATH}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${EXT_LIBS})
qt5_use_modules(${PROJECT_NAME} Quick Core Network Xml Widgets Sql)
