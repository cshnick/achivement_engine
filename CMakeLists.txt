cmake_minimum_required(VERSION 2.8.11)

#-fvisibility=internal
SET(CXX_COMMON
  "-std=c++11 -g -O0"
)
SET(C_COMMON
  "-g -O0"
)
SET(AE_SRC
  "../ae/src"
)
SET(DEF_COMMON
  -DENABLE_TESTS
  #-DNDEBUG
)
IF (NOT DEFINED PYTHON_BINDINGS_INSTALL_PATH)
  SET(PYTHON_BINDINGS_INSTALL_PATH
    "/home/ilia/Documents/script/python/achivement_engine"
  )
ENDIF (NOT DEFINED PYTHON_BINDINGS_INSTALL_PATH)
IF (NOT DEFINED QT_5_DIR)
  SET(QT_5_DIR
    "/home/ilia/Development/Qt/Binary/online/5.4/gcc_64"
  )
ENDIF (NOT DEFINED QT_5_DIR)
SET(BIN_PATH
  ${PROJECT_BINARY_DIR}/bin
)
IF (NOT DEFINED PYTHON_WRAPPER)
set(PYTHON_WRAPPER TRUE)
ENDIF (NOT DEFINED PYTHON_WRAPPER)

MESSAGE("Python bindings install path: ${PYTHON_BINDINGS_INSTALL_PATH}")
MESSAGE("Qt 5 directory: ${QT_5_DIR}")

set (AeCore ${CMAKE_CURRENT_BINARY_DIR}/bin/libAeCore.so)


add_subdirectory(Components)
