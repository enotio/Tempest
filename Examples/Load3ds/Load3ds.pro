TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

INCLUDEPATH += "C:/Users/Try/Home/Programming/SharedLibs/assimp/assimp/include"
LIBS += -L"C:/Users/Try/Home/Programming/SharedLibs/assimp/lib" -lassimp

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
    ../bin/data/post/combine.vs.glsl

