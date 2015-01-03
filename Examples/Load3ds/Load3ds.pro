TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

INCLUDEPATH += "../../../libs/assimp/include"
LIBS += -L"../../../libs/lib" -lassimp

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

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

