TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/shader/tess.fs.glsl \
    ../bin/shader/tess.vs.glsl \
    ../bin/shader/tess.ts.glsl \
    ../bin/shader/tess.es.glsl

