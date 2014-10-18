TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib/ -lTempest
LIBS        += -l"gdi32" -l"user32" -l"opengl32"

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/shader/basic.vs.glsl \
    ../bin/shader/basic.fs.glsl \
    ../bin/shader/basic.vs.gl4 \
    ../bin/shader/basic.fs.gl4

