include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

DESTDIR = ../bin

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib/ -lTempest

win32-g++: {
  #QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
  }

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

HEADERS +=    

SOURCES += \
    main.cpp \
    utils_test.cpp \
    layout_test.cpp \
    signals.cpp \
    enable_flag.cpp \
    widget_state.cpp \
    pixmap_test.cpp \
    textmodel_test.cpp \
    uniforms.cpp
