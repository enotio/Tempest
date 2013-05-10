QT       -= core gui
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += ogl directx

TARGET = Tempest
TEMPLATE = lib
#CONFIG += staticlib

#DEFINES += __ANDROID__
INCLUDEPATH += math
INCLUDEPATH += squish
INCLUDEPATH += "./include"
DESTDIR = ../lib

QMAKE_CXXFLAGS += -std=gnu++0x -Wall

INCLUDEPATH += \
               "$$(CG_INC_PATH)"\
               "$$(DEVIL_INC_PATH)"\
               "$$(GLEW_PATH)/include"\
               "./thirdparty/" \
               "."
#DEFINES += D3D_DEBUG_INFO

ogl:{
  LIBS += -L"$$(CG_LIB_PATH)" -l"opengl32" -l"cg" -l"cgGL"
  LIBS += -L"$$(GLEW_PATH)/lib"  -l"glew32" -l"glew32"

  HEADERS +=\
    ogl/opengl2x.h \
    ogl/cgogl.h \
    ogl/glsl.h \
    ogl/gltypes.h

  SOURCES += \
    ogl/opengl2x.cpp \
    ogl/cgogl.cpp \
    ogl/glsl.cpp \
    ogl/gltypes.cpp

  TARGET = Tempest_gl
  }

directx: {
  INCLUDEPATH += "$$(DXSDK_DIR)/include"
  LIBS += -L"$$(DXSDK_DIR)Lib/x86" -l"d3d9" -l"d3dx9"
  LIBS += -L"$$(CG_LIB_PATH)" -l"cg" -l"cgD3D9"

  HEADERS += \
    dx/cgdx9.h \
    dx/directx9.h

  SOURCES += \
    dx/cgdx9.cpp \
    dx/directx9.cpp

  TARGET = Tempest_dx
  }

ogl:{
  directx:{
    TARGET = Tempest
    }
  }

LIBS += -L"$$(DEVIL_LIB_PATH)" -l"DevIL"
LIBS += -l"gdi32"

SOURCES += \
    ui/window.cpp \
    system/windowsapi.cpp \
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
    system/androidapi.cpp \
    core/renderstate.cpp \
    core/device.cpp \
    core/abstractapi.cpp \
    core/wrappers/vertexdeclaration.cpp \
    core/wrappers/vertexbuffer.cpp \
    core/wrappers/texture2d.cpp \
    core/wrappers/pixmap.cpp \
    core/wrappers/indexbuffer.cpp \
    core/wrappers/atomic.cpp \
    dataControl/vertexshaderholder.cpp \
    dataControl/vertexbufferholder.cpp \
    dataControl/textureholder.cpp \
    dataControl/localvertexbufferholder.cpp \
    dataControl/localtexturesholder.cpp \
    dataControl/indexbufferholder.cpp \
    dataControl/fragmentshaderholder.cpp \
    dataControl/abstractholder.cpp \
    math/matrix4x4.cpp \
    render/render.cpp \
    scene/viewtester.cpp \
    scene/model.cpp \
    scene/lightcollection.cpp \
    scene/light.cpp \
    scene/graphicobject.cpp \
    scene/camera.cpp \
    scene/abstractscene.cpp \
    scene/abstractmaterial.cpp \
    scene/abstractlightcollection.cpp \
    scene/abstractgraphicobject.cpp \
    scene/abstractcamera.cpp \
    shading/vertexshader.cpp \
    shading/uniformtable.cpp \
    shading/uniformcash.cpp \
    shading/uniform.cpp \
    shading/fragmentshader.cpp \
    shading/abstractshadinglang.cpp \
    utils/tessobject.cpp \
    utils/postprocesshelper.cpp \
    utils/color.cpp \
    shading/shaderinput.cpp \
    squish/squish.cpp \
    squish/singlecolourlookup.inl \
    squish/singlecolourfit.cpp \
    squish/rangefit.cpp \
    squish/maths.cpp \
    squish/colourset.cpp \
    squish/colourfit.cpp \
    squish/colourblock.cpp \
    squish/clusterfit.cpp \
    squish/alpha.cpp \
    dataControl/resourcecontext.cpp \
    system/systemapi.cpp \
    core/wrappers/half.cpp \
    thirdparty/nv_math/nv_quat.cpp \
    thirdparty/nv_math/nv_matrix.cpp \
    thirdparty/nv_math/nv_math.cpp

HEADERS += \
    ui/window.h \
    system/windowsapi.h \
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
    system/androidapi.h \
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
    render/render.h \
    scene/viewtester.h \
    scene/model.h \
    scene/lightcollection.h \
    scene/light.h \
    scene/graphicobject.h \
    scene/camera.h \
    scene/abstractscene.h \
    scene/abstractmaterial.h \
    scene/abstractlightcollection.h \
    scene/abstractgraphicobject.h \
    scene/abstractcamera.h \
    shading/vertexshader.h \
    shading/uniformtable.h \
    shading/uniformcash.h \
    shading/uniform.h \
    shading/fragmentshader.h \
    shading/abstractshadinglang.h \
    utils/tessobject.h \
    utils/postprocesshelper.h \
    utils/cwnptr.h \
    utils/color.h \
    shading/shaderinput.h \
    squish/squish.h \
    squish/singlecolourfit.h \
    squish/simd_ve.h \
    squish/simd_sse.h \
    squish/simd_float.h \
    squish/simd.h \
    squish/rangefit.h \
    squish/maths.h \
    squish/config.h \
    squish/colourset.h \
    squish/colourfit.h \
    squish/colourblock.h \
    squish/clusterfit.h \
    squish/alpha.h \
    system/ddsdef.h \
    dataControl/resourcecontext.h \
    system/systemapi.h \
    core/wrappers/half.h \
    thirdparty/nv_math/NvVec.h \
    thirdparty/nv_math/nv_quat.h \
    thirdparty/nv_math/nv_matrix.h \
    thirdparty/nv_math/nv_math.h \
    thirdparty/nv_math/misc.h \
    thirdparty/GLES2/gl2.h

OTHER_FILES += \
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
    Android.mk \
    include/Tempest/ResourceContext \
    include/Tempest/SystemAPI \
    android/jni/Android.mk

