QT     -= core gui
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

CONFIG += ogl directx

msvc_static: {
  TEMPLATE = lib
  CONFIG   += staticlib
  } else {
  TEMPLATE = lib
  CONFIG   += dll
  }

ios:CONFIG   += staticlib

DEFINES += TEMPEST_LIBRARY_BUILD
DEFINES += TEMPEST_M_TREADS

INCLUDEPATH += "./include"

android:{
  DESTDIR = ../lib/$$ANDROID_TARGET_ARCH
  } else {
  DESTDIR = ../lib
  }

gcc:QMAKE_CXXFLAGS += -Wall -Wextra
#QMAKE_LFLAGS += /IMPLIB:Tempest.lib

INCLUDEPATH += \
               "./thirdparty/" \
               "."

win32:LIBS += -l"gdi32" -l"user32"

#DEFINES += D3D_DEBUG_INFO

android:{
  DEFINES += "sigset_t=\"unsigned int\""
  DEFINES += __STDC_INT64__
  DEFINES -= TEMPEST_M_TREADS

  CONFIG += ogl
  CONFIG -= directx
  SOURCES += system/androidapi.cpp
  }

unix: {
  DEFINES -= TEMPEST_M_TREADS
  CONFIG += ogl
  CONFIG -= directx
  SOURCES += system/linuxapi.cpp
  !android:!mac:LIBS += -lrt
  }

win32: {
  DEFINES += _CRT_SECURE_NO_WARNINGS
  DEFINES += NOMINMAX
  }

mac:!ios:{
  LIBS += -framework AppKit
  LIBS += -framework OpenGL
  LIBS += -framework CoreVideo
  }

ios:{
  LIBS += -framework Foundation
  LIBS += -framework UiKit
  LIBS += -framework GLKit
  LIBS += -framework OpenGLES
  QMAKE_IOS_DEPLOYMENT_TARGET = 5.0
  }

ogl:{
  win32:                 LIBS += -l"opengl32"
  unix: !android: !mac:  LIBS += -lX11 -lGL
  android:               LIBS += -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics -lz

  HEADERS +=\
    ogl/opengl2x.h \
    ogl/glsl.h \
    ogl/gltypes.h

  SOURCES += \
    ogl/opengl2x.cpp \
    ogl/glsl.cpp \
    ogl/gltypes.cpp

  !android:{
    HEADERS +=\
      ogl/opengl4x.h

    SOURCES += \
      ogl/opengl4x.cpp
    }

  TARGET = Tempest_gl
  }

directx: {
  INCLUDEPATH += "$$(DXSDK_DIR)/include"
  LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d9"  -l"d3dx9"
  LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d11" -l"d3dx11" -l"d3dcompiler"

  HEADERS += \
    dx/hlsl.h \
    dx/directx9.h \
    dx/directx11.h \
    dx/hlsl11.h

  SOURCES += \
    dx/hlsl.cpp \
    dx/hlsl11.cpp \
    dx/directx9.cpp \
    dx/directx11.cpp

  use_cg:{
    !isEmpty($$(CG_INC_PATH)):INCLUDEPATH += "$$(CG_INC_PATH)"

    HEADERS += \
      dx/cgdx9.h

    SOURCES += \
      dx/cgdx9.cpp

    !isEmpty($$(CG_LIB_PATH)):LIBS += -L"$$(CG_LIB_PATH)" -l"cg" -l"cgD3D9"
    }

  TARGET = Tempest_dx
  }

ogl:{
  directx:{
    TARGET = Tempest
    }
  }
android:TARGET = Tempest
unix   :TARGET = Tempest

include( thirdparty/libpng/libpng.pri )
include( thirdparty/libjpeg/libjpeg.pri )
include( thirdparty/squish/squish.pri )
include( thirdparty/freetype/freetype.pri )
include( thirdparty/nv_math/nv_math.pri )
include( thirdparty/utf8cpp/utf8cpp.pri )
include( thirdparty/ktx/ktx.pri )
!android:include( thirdparty/zlib/zlib.pri )
include( thirdparty/fakeGL/GLES2/gles.pri )

SOURCES += \
    system/windowsapi.cpp \
    system/systemapi.cpp \
    system/winphoneapi.cpp \
    core/graphicssubsystem.cpp \
    ui/window.cpp \
    ui/widget.cpp \
    utils/utility.cpp \
    ui/sizepolicy.cpp \
    ui/shortcut.cpp \
    ui/painter.cpp \
    ui/layout.cpp \
    ui/event.cpp \
    application.cpp \
    core/renderstate.cpp \
    core/device.cpp \
    core/abstractapi.cpp \
    core/wrappers/vertexdeclaration.cpp \
    core/wrappers/texture2d.cpp \
    core/wrappers/pixmap.cpp \
    core/wrappers/atomic.cpp \
    dataControl/vertexbufferholder.cpp \
    dataControl/textureholder.cpp \
    dataControl/localtexturesholder.cpp \
    dataControl/indexbufferholder.cpp \
    dataControl/abstractholder.cpp \
    math/matrix4x4.cpp \
    scene/viewtester.cpp \
    scene/model.cpp \
    scene/lightcollection.cpp \
    scene/light.cpp \
    scene/camera.cpp \
    scene/abstractcamera.cpp \
    shading/abstractshadinglang.cpp \
    utils/tessobject.cpp \
    utils/color.cpp \
    dataControl/resourcecontext.cpp \
    core/assert.cpp \
    2d/sprite.cpp \
    2d/spritesholder.cpp \
    timer.cpp \
    2d/surface.cpp \
    core/wrappers/displaysettings.cpp \
    2d/surfacerender.cpp \
    2d/font.cpp \
    ui/gesturerecognizer.cpp \
    core/wrappers/texture3d.cpp \
    dataControl/volumeholder.cpp \
    core/imagecodec.cpp \
    utils/log.cpp \
    io/iodevice.cpp \
    io/buffer.cpp \
    io/file.cpp \
    ogl/glfn.cpp \
    ogl/openglbase.cpp \
    dataControl/shaderprogramholder.cpp \
    shading/shaderprogram.cpp \
    ui/controls/button.cpp \
    ui/controls/uimetrics.cpp \
    ui/controls/panel.cpp \
    ui/controls/lineedit.cpp \
    ui/controls/listview.cpp \
    ui/controls/abstractlistbox.cpp \
    ui/controls/listbox.cpp \
    ui/controls/uifactory.cpp \
    ui/controls/dialog.cpp \
    ui/controls/checkbox.cpp \
    ui/controls/menu.cpp \
    ui/controls/stackedwidget.cpp \
    2d/icon.cpp \
    ui/controls/scrollbar.cpp \
    ui/controls/scrollwidget.cpp \
    ui/controls/label.cpp

HEADERS += \
    system/windowsapi.h \
    system/androidapi.h \
    system/systemapi.h \
    system/ddsdef.h \
    ui/window.h \
    ui/widget.h \
    utils/utility.h \
    ui/sizepolicy.h \
    ui/signal_slot.h \
    ui/shortcut.h \
    ui/painttextengine.h \
    ui/painter.h \
    ui/layout.h \
    ui/image.h \
    ui/event.h \
    application.h \
    core/renderstate.h \
    core/device.h \
    core/abstractapi.h \
    core/wrappers/vertexdeclaration.h \
    core/wrappers/vertexbuffer.h \
    core/wrappers/texture2d.h \
    core/wrappers/pixmap.h \
    core/wrappers/indexbuffer.h \
    core/wrappers/atomic.h \
    core/wrappers/abstracttexture.h \
    dataControl/vertexbufferholder.h \
    dataControl/textureholder.h \
    dataControl/localvertexbufferholder.h \
    dataControl/localtexturesholder.h \
    dataControl/indexbufferholder.h \
    dataControl/abstractholder.h \
    math/matrix4x4.h \
    scene/viewtester.h \
    scene/model.h \
    scene/lightcollection.h \
    scene/light.h \
    scene/graphicobject.h \
    scene/camera.h \
    scene/abstractscene.h \
    scene/abstractlightcollection.h \
    scene/abstractgraphicobject.h \
    scene/abstractcamera.h \
    shading/fragmentshader.h \
    shading/abstractshadinglang.h \
    utils/tessobject.h \
    utils/cwnptr.h \
    utils/color.h \
    dataControl/resourcecontext.h \
    core/wrappers/half.h \
    core/assert.h \
    2d/sprite.h \
    2d/spritesholder.h \
    timer.h \
    2d/surface.h \
    core/wrappers/displaysettings.h \
    2d/surfacerender.h \
    2d/font.h \
    dataControl/localobjectpool.h \
    ui/gesturerecognizer.h \
    core/wrappers/texture3d.h \
    dataControl/volumeholder.h \
    utils/mempool.h \
    core/imagecodec.h \
    utils/log.h \
    io/iodevice.h \
    io/buffer.h \
    io/file.h \
    ogl/glfn.h \
    core/graphicssubsystem.h \
    system/linuxapi.h \
    ogl/openglbase.h \
    shading/tessshader.h \
    shading/evalshader.h \
    core/platform.h \
    system/winphoneapi.h \
    utils/sortedvec.h \
    dataControl/shaderprogramholder.h \
    core/wrappers/uniformdeclaration.h \
    shading/shaderprogram.h \
    dx/dx11types.h \
    ui/controls/button.h \
    ui/controls/uimetrics.h \
    ui/controls/panel.h \
    ui/controls/lineedit.h \
    ui/controls/listview.h \
    ui/controls/listdelegate.h \
    ui/controls/abstractlistbox.h \
    ui/controls/listbox.h \
    ui/controls/uifactory.h \
    ui/controls/dialog.h \
    ui/controls/checkbox.h \
    ui/controls/menu.h \
    ui/controls/stackedwidget.h \
    2d/icon.h \
    ui/controls/scrollbar.h \
    ui/controls/scrollwidget.h \
    ui/controls/label.h \
    system/osxapi.h \
    system/appdelegate.h \
    system/iosapi.h

OTHER_FILES += \
    ../.gitignore \
    Android.mk \
    android/jni/Android.mk \
    ../README.md \
    Doxyfile \
    doc_title.doxy \
    ../doc/header.html \
    ../doc/footer.html \
    ../doc/html/style.css \
    include/Tempest/DirectX11 \
    include/Tempest/ShaderProgramHolder \
    include/Tempest/Windows \
    include/Tempest/IODevice \
    include/Tempest/Font \
    include/Tempest/SurfaceRender \
    include/Tempest/DisplaySettings \
    include/Tempest/LocalObjectPool \
    include/Tempest/Window \
    include/Tempest/Application \
    include/Tempest/Widget \
    include/Tempest/Utility \
    include/Tempest/SizePolicy \
    include/Tempest/signal \
    include/Tempest/Shortcut \
    include/Tempest/PaintTextEngine \
    include/Tempest/Painter \
    include/Tempest/Layout \
    include/Tempest/Image \
    include/Tempest/Event \
    include/Tempest/ResourceContext \
    include/Tempest/SystemAPI \
    include/Tempest/SpritesHolder \
    include/Tempest/Sprite \
    include/Tempest/Timer \
    include/Tempest/Surface \
    include/Tempest/Android \
    include/Tempest/GestureRecognizer \
    include/Tempest/Texture3d \
    include/Tempest/VolumeHolder \
    include/Tempest/ImageCodec \
    include/Tempest/Log \
    include/Tempest/Buffer \
    include/Tempest/File \
    include/Tempest/GraphicsSubsystem \
    include/Tempest/HLSL \
    include/Tempest/OpenGL4x \
    include/Tempest/Platform \
    include/Tempest/WindowsPhone \
    include/Tempest/UniformDeclaration \
    include/Tempest/UIMetrics \
    include/Tempest/Button \
    include/Tempest/Panel \
    include/Tempest/LineEdit \
    include/Tempest/ListDelegate \
    include/Tempest/ListView \
    include/Tempest/AbstractListBox \
    include/Tempest/ListBox \
    include/Tempest/ScrollBar \
    include/Tempest/ScrollWidget \
    include/Tempest/UiFactory \
    include/Tempest/Dialog \
    include/Tempest/CheckBox \
    include/Tempest/Menu \
    include/Tempest/StackedWidget \
    include/Tempest/Icon \
    include/Tempest/Label

OBJECTIVE_SOURCES += \
    system/osxapi.mm \
    system/appdelegate.mm \
    system/iosapi.mm
