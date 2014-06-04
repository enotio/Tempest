QT     -= core gui
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += ogl directx

TEMPLATE = lib
CONFIG   += dll

DEFINES += TEMPEST_M_TREADS

INCLUDEPATH += "./include"

android:{
  DESTDIR = ../lib/$$ANDROID_TARGET_ARCH
  }
else {
  DESTDIR = ../lib
  }

QMAKE_CXXFLAGS += -std=c++11 -Wall

INCLUDEPATH += \
               "./thirdparty/" \
               "."

win32:LIBS += -l"gdi32"
LIBS += -l"z"

#DEFINES += D3D_DEBUG_INFO

android:{
  DEFINES += "sigset_t=\"unsigned int\""
  DEFINES += __STDC_INT64__
  DEFINES -= TEMPEST_M_TREADS

  CONFIG += ogl
  CONFIG -= directx
  }

unix: {
  DEFINES -= TEMPEST_M_TREADS
  CONFIG += ogl
  CONFIG -= directx
  }

ogl:{
  win32:          LIBS += -l"opengl32"
  unix: !android: LIBS += -lX11 -lGL
  android:        LIBS += -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics

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
  LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d9" -l"d3dx9"

  HEADERS += \
    dx/hlsl.h \
    dx/directx9.h

  SOURCES += \
    dx/hlsl.cpp \
    dx/directx9.cpp

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
include( thirdparty/fakeGL/GLES2/gles.pri )

SOURCES += \
    system/windowsapi.cpp \
    system/systemapi.cpp \
    system/androidapi.cpp \
    core/graphicssubsystem.cpp \
    ui/window.cpp \
    ui/widget.cpp \
    utils/utility.cpp \
    ui/sizepolicy.cpp \
    ui/shortcut.cpp \
    ui/painttextengine.cpp \
    ui/painter.cpp \
    ui/layout.cpp \
    ui/image.cpp \
    ui/event.cpp \
    application.cpp \
    core/renderstate.cpp \
    core/device.cpp \
    core/abstractapi.cpp \
    core/wrappers/vertexdeclaration.cpp \
    core/wrappers/vertexbuffer.cpp \
    core/wrappers/texture2d.cpp \
    core/wrappers/pixmap.cpp \
    core/wrappers/indexbuffer.cpp \
    core/wrappers/atomic.cpp \
    dataControl/vertexbufferholder.cpp \
    dataControl/textureholder.cpp \
    dataControl/localvertexbufferholder.cpp \
    dataControl/localtexturesholder.cpp \
    dataControl/indexbufferholder.cpp \
    dataControl/abstractholder.cpp \
    math/matrix4x4.cpp \
    scene/viewtester.cpp \
    scene/model.cpp \
    scene/lightcollection.cpp \
    scene/light.cpp \
    scene/graphicobject.cpp \
    scene/camera.cpp \
    scene/abstractscene.cpp \
    scene/abstractlightcollection.cpp \
    scene/abstractgraphicobject.cpp \
    scene/abstractcamera.cpp \
    shading/vertexshader.cpp \
    shading/uniformcash.cpp \
    shading/fragmentshader.cpp \
    shading/abstractshadinglang.cpp \
    utils/tessobject.cpp \
    utils/color.cpp \
    shading/shaderinput.cpp \
    dataControl/resourcecontext.cpp \
    core/wrappers/half.cpp \
    core/assert.cpp \
    shading/shader.cpp \
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
    utils/mempool.cpp \
    shading/programobject.cpp \
    core/imagecodec.cpp \
    utils/log.cpp \
    io/iodevice.cpp \
    io/buffer.cpp \
    io/file.cpp \
    ogl/glfn.cpp \
    system/linuxapi.cpp \
    ogl/openglbase.cpp \
    shading/tessshader.cpp \
    shading/evalshader.cpp \
    core/devicesm5.cpp

HEADERS += \
    system/windowsapi.h \
    system/androidapi.h \
    system/systemapi.h \
    system/ddsdef.h \
    ui/window.h \
    ui/widget.h \
    utils/utility.h \
    ui/sizepolicy.h \
    ui/signal.h \
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
    dataControl/vertexshaderholder.h \
    dataControl/vertexbufferholder.h \
    dataControl/textureholder.h \
    dataControl/localvertexbufferholder.h \
    dataControl/localtexturesholder.h \
    dataControl/indexbufferholder.h \
    dataControl/fragmentshaderholder.h \
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
    shading/vertexshader.h \
    shading/uniformcash.h \
    shading/fragmentshader.h \
    shading/abstractshadinglang.h \
    utils/tessobject.h \
    utils/cwnptr.h \
    utils/color.h \
    shading/shaderinput.h \
    dataControl/resourcecontext.h \
    core/wrappers/half.h \
    core/assert.h \
    shading/shader.h \
    dataControl/shaderholder.h \
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
    shading/programobject.h \
    core/imagecodec.h \
    utils/log.h \
    io/iodevice.h \
    io/buffer.h \
    io/file.h \
    ogl/glfn.h \
    core/graphicssubsystem.h \
    system/platform.h \
    system/linuxapi.h \
    ogl/openglbase.h \
    shading/tessshader.h \
    dataControl/tessshaderholder.h \
    shading/evalshader.h \
    dataControl/evalshaderholder.h \
    core/devicesm5.h

OTHER_FILES += \
    ../.gitignore \
    Android.mk \
    android/jni/Android.mk \
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
    include/Tempest/TessShader \
    ../README.md \
    Doxyfile \
    doc_title.doxy \
    ../doc/header.html \
    ../doc/footer.html \
    ../doc/html/style.css \
    include/Tempest/OpenGL4x \
    include/Tempest/EvalShaderHolder \
    include/Tempest/EvalShader \
    include/Tempest/DeviceSM5
