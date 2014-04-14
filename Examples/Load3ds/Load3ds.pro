TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"


INCLUDEPATH += "C:/Users/Alexander/Home/Programming/libs/assimp/include"
LIBS += -L"C:/Users/Alexander/Home/Programming/libs/lib" -lassimp

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/data/main.fs.glsl \
    ../bin/data/main.vs.glsl \
    ../bin/data/blt.vs.glsl \
    ../bin/data/blt.fs.glsl \
    ../bin/data/post/gaus.fs.glsl \
    ../bin/data/post/gaus.vs.glsl \
    ../bin/data/post/bright.fs.glsl \
    ../bin/data/post/bright.vs.glsl \
    ../bin/data/post/combine.fs.glsl \
    ../bin/data/post/combine.vs.glsl \
    ../bin/shader/main.fs.glsl \
    ../bin/shader/main.vs.glsl \
    ../bin/shader/post/bright.fs.glsl \
    ../bin/shader/post/bright.vs.glsl \
    ../bin/shader/post/combine.fs.glsl \
    ../bin/shader/post/combine.vs.glsl \
    ../bin/shader/post/gaus.fs.glsl \
    ../bin/shader/post/gaus.vs.glsl

