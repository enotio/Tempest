TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../Tempest/include
LIBS += -L../../lib/ -lTempest

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/shader/basic.vs.hlsl \
    ../bin/shader/basic.fs.hlsl \
    ../bin/shader/basic11.fs.hlsl \
    ../bin/shader/basic11.vs.hlsl

