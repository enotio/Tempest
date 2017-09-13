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
    ../bin/shader/basic.vs.hlsl \
    ../bin/shader/basic.fs.hlsl \
    ../bin/shader/basic11.fs.hlsl \
    ../bin/shader/basic11.vs.hlsl

