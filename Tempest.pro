QT     -= core gui
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += ogl directx

TARGET   = Tempest
TEMPLATE = lib
CONFIG   += dll

DEFINES += TEMPEST_M_TREADS
#DEFINES += __ANDROID__

INCLUDEPATH += math
INCLUDEPATH += squish
INCLUDEPATH += "./include"
DESTDIR = ../lib

QMAKE_CXXFLAGS += -std=gnu++0x -Wall

INCLUDEPATH += \
               "$$(CG_INC_PATH)"\
               "C:/Users/Try/Home/Programming/SharedLibs/glew-1.5.4-mingw32/include"\
               "./thirdparty/fakeGL" \
               "./thirdparty/" \
               "."

INCLUDEPATH += "C:/Users/Try/Home/adt-bundle-windows-x86-20130729/android-ndk-r9b/platforms/android-9/arch-arm/usr/include/android"
INCLUDEPATH += "C:/Users/Try/Home/Programming/SharedLibs/freetype-dev_2.4.2-1_win32/include"
INCLUDEPATH += "C:/Users/Try/Home/Programming/SharedLibs/freetype-dev_2.4.2-1_win32/include/freetype2"

LIBS += -L"C:/Users/Try/Home/Programming/SharedLibs/freetype-dev_2.4.2-1_win32/lib" -lfreetype
LIBS += -l"gdi32" -l"z"

#DEFINES += D3D_DEBUG_INFO

ogl:{
  LIBS += -L"$$(CG_LIB_PATH)"
  LIBS += -L"C:/Users/Try/Home/Programming/SharedLibs/glew-1.5.4-mingw32/lib"

  LIBS += -l"opengl32" -l"cg" -l"cgGL" -l"glew32"

  HEADERS +=\
    ogl/opengl2x.h \
    ogl/glsl.h \
    ogl/gltypes.h

  SOURCES += \
    ogl/opengl2x.cpp \
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
    dataControl/vertexbufferholder.cpp \
    dataControl/textureholder.cpp \
    dataControl/localvertexbufferholder.cpp \
    dataControl/localtexturesholder.cpp \
    dataControl/indexbufferholder.cpp \
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
    shading/fragmentshader.cpp \
    shading/abstractshadinglang.cpp \
    utils/tessobject.cpp \
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
    thirdparty/nv_math/nv_math.cpp \
    thirdparty/libpng/pngwutil.c \
    thirdparty/libpng/pngwtran.c \
    thirdparty/libpng/pngwrite.c \
    thirdparty/libpng/pngwio.c \
    thirdparty/libpng/pngtrans.c \
    thirdparty/libpng/pngset.c \
    thirdparty/libpng/pngrutil.c \
    thirdparty/libpng/pngrtran.c \
    thirdparty/libpng/pngrio.c \
    thirdparty/libpng/pngread.c \
    thirdparty/libpng/pngpread.c \
    thirdparty/libpng/pngmem.c \
    thirdparty/libpng/pngget.c \
    thirdparty/libpng/pngerror.c \
    thirdparty/libpng/png.c \
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
    thirdparty/ktx/etc_dec.cpp \
    utils/log.cpp \
    io/iodevice.cpp \
    io/buffer.cpp \
    thirdparty/libjpeg/jaricom.c \
    thirdparty/libjpeg/jcomapi.c \
    thirdparty/libjpeg/jdapimin.c \
    thirdparty/libjpeg/jdapistd.c \
    thirdparty/libjpeg/jdarith.c \
    thirdparty/libjpeg/jdatadst.c \
    thirdparty/libjpeg/jdatasrc.c \
    thirdparty/libjpeg/jdcoefct.c \
    thirdparty/libjpeg/jdcolor.c \
    thirdparty/libjpeg/jddctmgr.c \
    thirdparty/libjpeg/jdhuff.c \
    thirdparty/libjpeg/jdinput.c \
    thirdparty/libjpeg/jdmainct.c \
    thirdparty/libjpeg/jdmarker.c \
    thirdparty/libjpeg/jdmaster.c \
    thirdparty/libjpeg/jdmerge.c \
    thirdparty/libjpeg/jdpostct.c \
    thirdparty/libjpeg/jdsample.c \
    thirdparty/libjpeg/jdtrans.c \
    thirdparty/libjpeg/jerror.c \
    thirdparty/libjpeg/jidctflt.c \
    thirdparty/libjpeg/jidctfst.c \
    thirdparty/libjpeg/jidctint.c \
    thirdparty/libjpeg/jmemmgr.c \
    thirdparty/libjpeg/jmemnobs.c \
    thirdparty/libjpeg/jquant1.c \
    thirdparty/libjpeg/jquant2.c \
    thirdparty/libjpeg/jutils.c \
    io/file.cpp

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
    shading/fragmentshader.h \
    shading/abstractshadinglang.h \
    utils/tessobject.h \
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
    thirdparty/fakeGL/GLES2/gl2.h \
    thirdparty/libpng/pngpriv.h \
    thirdparty/libpng/pngconf.h \
    thirdparty/libpng/png.h \
    thirdparty/libpng/config.h \
    core/assert.h \
    shading/shader.h \
    dataControl/shaderholder.h \
    2d/sprite.h \
    2d/spritesholder.h \
    timer.h \
    2d/surface.h \
    core/wrappers/displaysettings.h \
    include/Tempest/DisplaySettings \
    2d/surfacerender.h \
    include/Tempest/SurfaceRender \
    2d/font.h \
    include/Tempest/Font \
    dataControl/localobjectpool.h \
    include/Tempest/LocalObjectPool \
    ui/gesturerecognizer.h \
    core/wrappers/texture3d.h \
    dataControl/volumeholder.h \
    utils/mempool.h \
    shading/programobject.h \
    core/imagecodec.h \
    thirdparty/ktx/etc_dec.h \
    utils/log.h \
    io/iodevice.h \
    io/buffer.h \
    include/Tempest/IODevice \
    thirdparty/libjpeg/jconfig.h \
    thirdparty/libjpeg/jdct.h \
    thirdparty/libjpeg/jerror.h \
    thirdparty/libjpeg/jinclude.h \
    thirdparty/libjpeg/jmemsys.h \
    thirdparty/libjpeg/jmorecfg.h \
    thirdparty/libjpeg/jpegint.h \
    thirdparty/libjpeg/jpeglib.h \
    thirdparty/libjpeg/jversion.h \
    io/file.h

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
    android/jni/Android.mk \
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
    thirdparty/freetype/Android.mk \
    include/Tempest/Buffer \
    include/Tempest/File

