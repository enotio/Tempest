TEMPLATE = app
CONFIG += console
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
    ../bin/shader/basic.vs.glsl \
    ../bin/shader/basic.fs.glsl
