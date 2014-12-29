TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../Tempest/include
LIBS += -L../../lib/ -lTempest

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d9 -ld3dx9 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

