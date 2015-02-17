cmake_minimum_required(VERSION 2.8.11)

project(Axc)
set(CMAKE_PREFIX_PATH ${QT_5_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET (SRC_DIR "src")
SET (HIGHLIGHTER_DIR ${SRC_DIR}/highlighter)
include_directories(
	.
	${SRC_DIR}
	${AE_SRC}
	${HIGHLIGHTER_DIR}
)
SET (SRC_LIST
  ${SRC_DIR}/xmllistmodel.cpp
  ${SRC_DIR}/main.cpp
  ${HIGHLIGHTER_DIR}/highlighter.cpp
  ${SRC_DIR}/string_encoder.cpp
)

SET (HEADERS
  ${SRC_DIR}/xmllistmodel.h
  ${HIGHLIGHTER_DIR}/highlighter.h
)
find_package(Qt5 COMPONENTS Quick Core Network Xml Widgets)
qt5_add_resources(RESOURCES qml.qrc)
qt5_wrap_cpp(xmllistmodel_MOC ${SRC_DIR}/xmllistmodel.h)
qt5_wrap_cpp(highlighter_MOC ${HIGHLIGHTER_DIR}/highlighter.h)
qt5_wrap_cpp(string_encoder_MOC ${SRC_DIR}/string_encoder.h)

add_executable(${PROJECT_NAME}
	${SRC_LIST} 
	${xmllistmodel_MOC}
	${highlighter_MOC}
	${string_encoder_MOC}
	${RESOURCES} 
	${HEADERS}
)

#copy to binary directory
ADD_CUSTOM_COMMAND( 
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND /usr/bin/mkdir -p ${BIN_PATH}
   COMMAND /usr/bin/cp -rvf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${BIN_PATH}
)


qt5_use_modules(${PROJECT_NAME} Quick Core Network Xml Widgets)