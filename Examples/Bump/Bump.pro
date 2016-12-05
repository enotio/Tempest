TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

DESTDIR = ../bin

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

win32-g++: {
  QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
  }

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

