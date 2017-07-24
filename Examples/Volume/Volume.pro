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
    ../bin/shader/volume.fs.glsl \
    ../bin/shader/volume.vs.glsl


