INCLUDEPATH += ../../Tempest/include
LIBS += -L../../lib/ -lTempest

DESTDIR = ../bin

win32-g++: {
  QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
  }

win32:{
  #msvc static build
  contains(QT_ARCH, i386):{
    LIBS += -L"$$(DXSDK_DIR)Lib/x86"
    } else {
    LIBS += -L"$$(DXSDK_DIR)Lib/x64"
    }
  LIBS += -luser32 -lgdi32 -ld3d9 -ld3dx9 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32 -ldxguid
  }
