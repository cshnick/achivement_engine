TEMPLATE = app

QT += qml quick widgets xml
SRC_DIR = src
INCLUDEPATH += $$SRC_DIR \
               $$SRC_DIR/highlighter

SOURCES += $$SRC_DIR/main.cpp \
    $$SRC_DIR/xmllistmodel.cpp \
    src/highlighter/highlighter.cpp \
    src/string_encoder.cpp

RESOURCES += qml.qrc
CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =
isEmpty(AE_SRC) : AE_SRC="../Components/ae/src"

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    $$SRC_DIR/xmllistmodel.h \
    $$SRC_DIR/highlighter/highlighter.h \
    src/string_encoder.h

INCLUDEPATH += $$AE_SRC
