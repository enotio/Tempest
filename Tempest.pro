QT       -= core gui

CONFIG += opengl directx

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
               "."
#DEFINES += D3D_DEBUG_INFO

opengl:{
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

opengl:{
  directx:{
    TARGET = Tempest
    }
  }

LIBS += -L"$$(DEVIL_LIB_PATH)" -l"DevIL"
LIBS += -l"gdi32"

SOURCES += \
    window.cpp \
    system/abstractsystemapi.cpp \
    system/windowsapi.cpp \
    widget.cpp \
    utility.cpp \
    sizepolicy.cpp \
    shortcut.cpp \
    painttextengine.cpp \
    painter.cpp \
    layout.cpp \
    image.cpp \
    event.cpp \
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
    squish/alpha.cpp

HEADERS += \
    window.h \
    system/abstractsystemapi.h \
    system/windowsapi.h \
    widget.h \
    utility.h \
    sizepolicy.h \
    signal.h \
    shortcut.h \
    painttextengine.h \
    painter.h \
    layout.h \
    image.h \
    event.h \
    application.h \
    system/androidapi.h \
    system/STLConfig.h \
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
    squish/alpha.h

OTHER_FILES += \
    include/Tempest/Window \
    include/Tempest/Application \
    include/Tempest/AbstractSystemAPI \
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
    Android.mk

