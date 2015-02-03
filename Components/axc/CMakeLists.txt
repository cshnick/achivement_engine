cmake_minimum_required(VERSION 2.8.11)

project(Axc)
set(CMAKE_PREFIX_PATH ${QT_5_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET (SRC_DIR "src")
include_directories(
	.
	${SRC_DIR}
	${AE_SRC}
)
SET (SRC_LIST
  ${SRC_DIR}/xmllistmodel.cpp
  ${SRC_DIR}/main.cpp
)

SET (HEADERS
  ${SRC_DIR}/xmllistmodel.h
)
find_package(Qt5 COMPONENTS Quick Core Network Xml Widgets)
qt5_add_resources(RESOURCES qml.qrc)
qt5_wrap_cpp(xmllistmodel_MOC ${SRC_DIR}/xmllistmodel.h)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${xmllistmodel_MOC} ${RESOURCES} ${HEADERS})

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${BIN_PATH}
)


qt5_use_modules(${PROJECT_NAME} Quick Core Network Xml Widgets)