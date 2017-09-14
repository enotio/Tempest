TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

include(../common.pri)

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/shader/tess.fs.glsl \
    ../bin/shader/tess.vs.glsl \
    ../bin/shader/tess.ts.glsl \
    ../bin/shader/tess.es.glsl

