TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

DESTDIR = ../bin

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

SOURCES += main.cpp

