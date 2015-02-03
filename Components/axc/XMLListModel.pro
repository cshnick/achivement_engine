TEMPLATE = app

QT += qml quick widgets xml
SRC_DIR = src
INCLUDEPATH += $$SRC_DIR

SOURCES += $$SRC_DIR/main.cpp \
    $$SRC_DIR/xmllistmodel.cpp

RESOURCES += qml.qrc
CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =
isEmpty(AE_SRC) : AE_SRC="/home/ilia/Development/achivement_engine/src"

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    $$SRC_DIR/xmllistmodel.h

INCLUDEPATH += $$AE_SRC
