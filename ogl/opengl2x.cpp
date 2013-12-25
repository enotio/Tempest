#include "opengl2x.h"

#ifdef __ANDROID__
//GL_HALF_FLOAT -> GL_HALF_FLOAT_OES -> 0x8D61
#define GL_HALF_FLOAT 0x8D61

//#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#else
#include <windows.h>

#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <Tempest/GLSL>
#include <Tempest/RenderState>
#include <Tempest/Log>

#include <map>
#include <set>
#include <iostream>
#include <Tempest/Pixmap>

#include <Tempest/Assert>
#include <stdexcept>

#include "gltypes.h"
#include <squish.h>
#include <cstring>

#include "../utils/mempool.h"

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#endif

#define OGL_DEBUG

#ifdef __ANDROID__
#include "system/androidapi.h"
#endif

#define GL_COLOR_BUFFER_BIT0_QCOM                     0x00000001
#define GL_DEPTH_BUFFER_BIT0_QCOM                     0x00000100
#define GL_STENCIL_BUFFER_BIT0_QCOM                   0x00010000

#define GL_WRITEONLY_RENDERING_QCOM                   0x8823

#ifndef GL_COLOR_EXT
#define GL_COLOR_EXT                                  0x1800
#define GL_DEPTH_EXT                                  0x1801
#define GL_STENCIL_EXT                                0x1802
#endif
//#define GL_MAX_VARYING_VECTORS                        0x8DFC

using namespace Tempest;
typedef void (*PFNGLSTARTTILINGQCOMPROC) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
typedef void (*PFNGLENDTILINGQCOMPROC) (GLbitfield preserveMask);
typedef void (*PFNGLWGLSWAPINTERVALPROC) (GLint interval);

typedef void (*PFNGLDISCARDFRAMEBUFFERPROC)(GLenum mode, GLsizei count, const GLenum* att );

struct Opengl2x::Device{
#ifdef __ANDROID__
  EGLDisplay disp;
  EGLSurface s;
  EGLint     swapEfect;
#else
  HDC   hDC;
  HGLRC hRC;
#endif
  bool hasS3tcTextures,
       hasETC1Textures,
       hasHalfSupport,
       hasWriteonlyRendering;

  int scrW, scrH;
  AbstractAPI::Caps caps;

  VertexDeclaration::Declarator * decl;
  GLuint vbo, curVBO;
  GLuint ibo, curIBO;

  std::vector<bool> vAttrLoc;

  int vertexSize, curVboOffsetIndex, curIboOffsetIndex;

  GLuint fboId;
  bool   isPainting;

  Tempest::RenderState renderState;

  Detail::RenderTg target;
  struct FBOs{
    FBOs(){
      buckets.reserve(16);
      }

    struct Bucket{
      int count;
      std::vector<Detail::GLTexture*> targets;
      std::vector<int>    mip;
      std::vector<GLuint> fbo;
      };

    std::vector<Bucket> buckets;

    void onDeleteTexture( Detail::GLTexture* t ){
      for( size_t i=0; i<buckets.size(); ++i )
        onDeleteTexture( buckets[i], t );
      }

    void onDeleteTexture( Bucket &b, Detail::GLTexture* t ){
      int tgCount = b.count+1;
      for( size_t i=0, fboI = 0; i<b.targets.size();  ){
        bool ok = false;
        for( int r=0; r<tgCount; ++r )
          ok |= (b.targets[i+r]==t);

        if( ok ){
          glDeleteFramebuffers(1, &b.fbo[fboI]);

          b.fbo[fboI] = b.fbo.back();
          b.fbo.pop_back();

          size_t bsz = b.targets.size()-tgCount;
          for( int r=0; r<tgCount; ++r )
            b.targets[i+r] = b.targets[bsz+r];

          for( int r=0; r<tgCount; ++r )
            b.mip[i+r] = b.mip[bsz+r];

          b.targets.resize( b.targets.size()-tgCount );
          b.mip.resize( b.targets.size() );
          } else {
          i+=tgCount;
          fboI++;
          }
        }
      }

    bool cmpTagets( const Detail::RenderTg& tg,
                    Detail::GLTexture** rtg,
                    int *mip,
                    int sz ){
      bool ok = 1;
      for( int r=0; ok && r<sz; ++r )
        ok &=( tg.color[r]==rtg[r] && tg.mip[r]==mip[r]);

      ok &= ( tg.depth == rtg[sz] && tg.depthMip==mip[sz] );

      return ok;
      }

    GLuint& getTarget( const Detail::RenderTg& tg, int sz, size_t& hash ){
      Bucket &b = bucket(sz);
      int tgCount = sz+1;

      if( hash < b.fbo.size() &&
          cmpTagets(tg, &b.targets[hash*tgCount], &b.mip[hash*tgCount], sz ) )
        return b.fbo[hash];

      for( size_t i=0; i<b.targets.size(); i+=tgCount ){
        if( cmpTagets(tg, &b.targets[i], &b.mip[i], sz ) ){
          hash = i/tgCount;
          return b.fbo[hash];
          }
        }

      size_t i0 = b.targets.size();
      b.targets.resize( b.targets.size()+tgCount );
      for( size_t i=0; i<size_t(sz); ++i )
        b.targets[i0+i] = tg.color[i];
      b.targets.back() = tg.depth;

      b.mip.resize( b.targets.size() );
      for( size_t i=0; i<size_t(sz); ++i )
        b.mip[i0+i] = tg.mip[i];
      b.mip.back() = tg.depthMip;

      hash = b.fbo.size();
      b.fbo.push_back(0);
      return b.fbo.back();
      }

    Bucket& bucket( int mrt ){
      for( size_t i=0; i<buckets.size(); ++i ){
        if( buckets[i].count==mrt )
          return buckets[i];

        if( buckets[i].count>mrt ){
          buckets.emplace( buckets.begin()+i );
          return buckets[i];
          }
        }

      buckets.emplace_back();
      buckets.back().count = mrt;
      return buckets.back();
      }
    } fbo;

  std::vector<char> tmpLockBuffer;
  bool lbUseed;

  inline static uint32_t nextPot( uint32_t v ){
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
    }

  bool hasAlpha( AbstractTexture::Format::Type f ){
    if( AbstractTexture::Format::RGBA<=f &&
        f<=AbstractTexture::Format::RGBA16 )
      return true;

    if( AbstractTexture::Format::RGBA_DXT1<=f &&
        f<=AbstractTexture::Format::RGBA_DXT5 )
      return true;

    return false;
    }

  void texFormat( AbstractTexture::Format::Type f,
                  GLenum& storage,
                  GLenum& inFrm,
                  GLenum& inBytePkg,
                  bool renderable ){

#ifndef __ANDROID__
  static const GLenum format[] = {
    GL_LUMINANCE16,
    GL_LUMINANCE4_ALPHA4,
    GL_LUMINANCE8,
    GL_LUMINANCE16,

    GL_RGB8,
    GL_RGB4,
    GL_RGB5,
    GL_RGB10,
    GL_RGB12,
    GL_RGB16,

    GL_RGBA,
    GL_RGB5_A1,
    GL_RGBA8,
    GL_RGB10_A2,
    GL_RGBA12,
    GL_RGBA16,

    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

    GL_DEPTH_COMPONENT, //16
    GL_DEPTH_COMPONENT, //24
    GL_DEPTH_COMPONENT, //32
    GL_DEPTH_STENCIL,

    GL_RG16,
    GL_RGBA
    };
#else
  static const GLenum format[] = {
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_LUMINANCE,
    GL_LUMINANCE,

    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,

    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    GL_LUMINANCE,
    GL_LUMINANCE,
    GL_LUMINANCE,
    GL_LUMINANCE,

    GL_RGB,
    GL_RGBA
    };
#endif

    static const GLenum inputFormat[] = {
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_LUMINANCE,
    GL_LUMINANCE,

    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,

    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    GL_DEPTH_COMPONENT, //d
    GL_DEPTH_COMPONENT, //d
    GL_DEPTH_COMPONENT, //d
    GL_DEPTH_COMPONENT, //ds

    GL_RGB,
    GL_RGBA
    };

    if( !hasRenderToRGBTextures && renderable ){
      if( hasAlpha(f) )
        f = AbstractTexture::Format::RGBA5; else
        f = AbstractTexture::Format::RGB5;
      }

    inBytePkg = GL_UNSIGNED_BYTE;
#ifdef __ANDROID__
    if( f==AbstractTexture::Format::RGB ||
        f==AbstractTexture::Format::RGB4 )
      inBytePkg = GL_UNSIGNED_SHORT_5_6_5;

    if( f==AbstractTexture::Format::RGB5 )
      inBytePkg = GL_UNSIGNED_SHORT_5_6_5;

    if( f==AbstractTexture::Format::RGBA5 ||
        f==AbstractTexture::Format::RGBA )
      inBytePkg = GL_UNSIGNED_SHORT_5_5_5_1;
#endif

    storage = format[f];
#ifdef __ANDROID__
    inFrm = storage;
#else
    inFrm = inputFormat[f];
#endif
    }

  static const char* glErrorDesc( GLenum err ){
    struct Err{
      const char* str;
      GLenum code;
      };

    Err e[] = {
      {"GL_INVALID_ENUM",      GL_INVALID_ENUM},
      {"GL_INVALID_VALUE",     GL_INVALID_VALUE},
      {"GL_INVALID_OPERATION", GL_INVALID_OPERATION},
      {"GL_STACK_OVERFLOW",    GL_STACK_OVERFLOW},
      {"GL_STACK_UNDERFLOW",   GL_STACK_UNDERFLOW},
      {"GL_OUT_OF_MEMORY",     GL_OUT_OF_MEMORY},
      {"GL_INVALID_FRAMEBUFFER_OPERATION", GL_INVALID_FRAMEBUFFER_OPERATION},
      {"", GL_NO_ERROR},
    };

    for( int i=0; e[i].code!=GL_NO_ERROR; ++i )
      if( e[i].code==err )
        return e[i].str;

    return 0;
    }

  Color clearColor;
  float clearDepth;
  unsigned  clearS;

  bool hasTileBasedRender, hasQCOMTiles, hasDiscardBuffers;
  bool hasRenderToRGBTextures, hasNpotTexture;

  PFNGLSTARTTILINGQCOMPROC glStartTilingQCOM;
  PFNGLENDTILINGQCOMPROC   glEndTilingQCOM;
  PFNGLWGLSWAPINTERVALPROC wglSwapInterval;

  PFNGLDISCARDFRAMEBUFFERPROC glDiscardFrameBuffer;
  bool isTileRenderStarted;

  MemPool<Detail::GLTexture> texPool;
  MemPool<Detail::GLBuffer>  bufPool;
  MemPool<VertexDeclaration::Declarator> declPool;

  bool isWriteOnly( const RenderState& rs ){
    bool w[4];
    rs.getColorMask( w[0], w[1], w[2], w[3] );
    return !rs.isZTest() &&
           !rs.isZWriting() &&
           !rs.isBlend() &&
           w[0] && w[1] && w[2] && w[3];
    }

  void enableWriteOnlyRender( bool e ){
    if( 0 && hasWriteonlyRendering ){
      if( e )
        glEnable ( GL_WRITEONLY_RENDERING_QCOM ); else
        glDisable( GL_WRITEONLY_RENDERING_QCOM );
      }
    }
  };

bool Opengl2x::errCk() const {
#ifdef OGL_DEBUG
  GLenum err = glGetError();
  bool ok = true;

  while( err!=GL_NO_ERROR ){
    const char* glErrorDesc = Device::glErrorDesc(err);
#ifndef __ANDROID__
    std::cout << "[OpenGL]: ";
    if( glErrorDesc )
      std::cout << glErrorDesc <<" ";
    std::cout  <<"0x"<< std::hex << err << std::dec << std::endl;
#else
    void* ierr = (void*)err;
    if( glErrorDesc )
      __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "error %s", glErrorDesc); else
      __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "error %p", ierr);
#endif
    err = glGetError();
    ok = false;
    }
#endif

  return ok;
  }

Opengl2x::Opengl2x(){
  }

Opengl2x::~Opengl2x(){
  }

Opengl2x::Caps Opengl2x::caps( AbstractAPI::Device* d ) const {
  Device * dev = (Device*)d;
  return dev->caps;
  }

std::string Opengl2x::vendor( AbstractAPI::Device* d ) const {
  if( !setDevice(d) ) return "";

  const GLubyte *s = glGetString(GL_VENDOR);
  return (const char*)s;
  }

std::string Opengl2x::renderer(AbstractAPI::Device *d) const {
  if( !setDevice(d) ) return "";

  const GLubyte *s = glGetString(GL_RENDERER);
  return (const char*)s;
  }

AbstractAPI::Device* Opengl2x::createDevice(void *hwnd, const Options &opt) const {
  Device * dev = new Device();
  dev->isPainting = false;
  memset( &dev->target, 0, sizeof(dev->target) );
  dev->lbUseed = false;
  dev->tmpLockBuffer.reserve( 8096*32 );

#ifndef __ANDROID__
  GLuint PixelFormat;

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

  pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion   = 1;
  pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;

  HDC hDC = GetDC( (HWND)hwnd );
  PixelFormat = ChoosePixelFormat( hDC, &pfd );
  SetPixelFormat( hDC, PixelFormat, &pfd);

  HGLRC hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );

  dev->hDC = hDC;
  dev->hRC = hRC;

  dev->wglSwapInterval = 0;

  if( !GLEW_VERSION_2_1 )
    if( glewInit()!=GLEW_OK || !GLEW_VERSION_2_1) {
      return 0;
      }
#else
  dev->disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  dev->s    = eglGetCurrentSurface(EGL_DRAW);

  dev->swapEfect = EGL_BUFFER_PRESERVED;
#endif

  const char * ext = (const char*)glGetString(GL_EXTENSIONS);
  dev->hasS3tcTextures =
      (strstr(ext, "GL_OES_texture_compression_S3TC")!=0) ||
      (strstr(ext, "GL_EXT_texture_compression_s3tc")!=0);
  dev->hasETC1Textures = (strstr(ext, "GL_OES_compressed_ETC1_RGB8_texture")!=0);
  dev->hasWriteonlyRendering = (strstr(ext, "GL_QCOM_writeonly_rendering")!=0);

  dev->hasNpotTexture = (strstr(ext, "GL_OES_texture_npot")!=0) ||
                        (strstr(ext, "GL_ARB_texture_non_power_of_two")!=0);
  //dev->hasNpotTexture = 0;

#ifdef __ANDROID__
  dev->hasHalfSupport            = strstr(ext, "GL_OES_vertex_half_float")!=0;
  dev->hasRenderToRGBTextures    = strstr(ext, "GL_OES_rgb8_rgba8")!=0;
#else
  dev->hasHalfSupport             = 1;
  dev->hasRenderToRGBTextures     = 1;
#endif

  dev->hasQCOMTiles      = strstr(ext, "GL_QCOM_tiled_rendering")!=0;
  dev->hasDiscardBuffers = strstr(ext, "GL_EXT_discard_framebuffer")!=0;

#ifdef __WIN32
  if( strstr(ext, "WGL_EXT_swap_control") ){
    dev->wglSwapInterval = (PFNGLWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
    }
#endif

  dev->isTileRenderStarted = false;

  dev->hasTileBasedRender = dev->hasQCOMTiles | dev->hasDiscardBuffers;

#ifdef __ANDROID__
  if( dev->hasQCOMTiles ){
    dev->glStartTilingQCOM = (PFNGLSTARTTILINGQCOMPROC)eglGetProcAddress("glStartTilingQCOM");
    dev->glEndTilingQCOM   =   (PFNGLENDTILINGQCOMPROC)eglGetProcAddress("glEndTilingQCOM");
    }

  if( dev->hasDiscardBuffers ){
    dev->glDiscardFrameBuffer = (PFNGLDISCARDFRAMEBUFFERPROC)eglGetProcAddress("glDiscardFramebufferEXT");
    }
#endif

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "vendor = %s", glGetString(GL_VENDOR) );
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "render = %s", glGetString(GL_RENDERER) );
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "extensions = %s", ext );
#endif

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CW );

  dev->renderState.setCullFaceMode( RenderState::CullMode::noCull );
  dev->renderState.setZTest(0);
  dev->renderState.setZWriting(1);
  dev->renderState.setAlphaTestMode( RenderState::AlphaTestMode::Count );
  dev->renderState.setBlend(0);

  dev->clearColor = Color(0);
  dev->clearDepth = 1;
  dev->clearS     = 0;

  memset( (char*)&dev->caps, 0, sizeof(dev->caps) );
  // T_WARNING_X( dev->hasHalfSupport, "half_float not aviable on device" );
  dev->caps.hasHalf2 = dev->hasHalfSupport;
  dev->caps.hasHalf4 = dev->hasHalfSupport;
  T_ASSERT_X( errCk(), "OpenGL error" );
  glGetIntegerv( GL_MAX_TEXTURE_SIZE,         &dev->caps.maxTextureSize );
  T_ASSERT_X( errCk(), "OpenGL error" );
#ifdef __ANDROID__
  glGetIntegerv( GL_MAX_VARYING_VECTORS,      &dev->caps.maxVaryingVectors );
  dev->caps.maxVaryingComponents = dev->caps.maxVaryingVectors*4;
#else
  glGetIntegerv( GL_MAX_VARYING_COMPONENTS,   &dev->caps.maxVaryingComponents );
  dev->caps.maxVaryingVectors = dev->caps.maxVaryingComponents/4;
#endif
  T_ASSERT_X( errCk(), "OpenGL error" );

#ifdef __ANDROID__
  dev->caps.maxRTCount = 1;
#else
  glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &dev->caps.maxRTCount );
  if( dev->caps.maxRTCount>32 )
    dev->caps.maxRTCount = 32;
#endif
  dev->caps.hasNpotTexture = dev->hasNpotTexture;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenFramebuffers(1, &dev->fboId);

  reset( (AbstractAPI::Device*)dev, hwnd, opt );
  setRenderState( (AbstractAPI::Device*)dev, Tempest::RenderState() );

  T_ASSERT_X( errCk(), "OpenGL error" );
  return (AbstractAPI::Device*)dev;
  }

void Opengl2x::deleteDevice(AbstractAPI::Device *d) const {
  Device* dev = (Device*)d;
  if( !setDevice(d) ) return;
  glDeleteFramebuffers (1, &dev->fboId);

#ifndef __ANDROID__
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( dev->hRC );
#endif

  delete dev;
  }

bool Opengl2x::setDevice( AbstractAPI::Device * d ) const {
  dev = (Device*)d;
  T_ASSERT_X( errCk(), "OpenGL error" );

#ifdef __ANDROID__
  SystemAPI::GraphicsContexState state = SystemAPI::NotAviable;
  while( state==SystemAPI::NotAviable )
    state = SystemAPI::instance().isGraphicsContextAviable(0);

  if( state==SystemAPI::DestroyedByAndroid )
    __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "context is unaviable - android sucks");
  return state == SystemAPI::Aviable;
#endif

  return 1;
  }

void Opengl2x::clear( AbstractAPI::Device *d,
                      const Color& cl, float z, unsigned stencil ) const {
  if( !setDevice(d) ) return;

  if( dev->clearColor!=cl ){
    glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
    dev->clearColor = cl;
    }

  if( dev->clearDepth!=z ){
    dev->clearDepth = z;
  #ifdef __ANDROID__
    glClearDepthf( z );
  #else
    glClearDepth( z );
  #endif
    }

  if( dev->clearS!=stencil ){
    glClearStencil( stencil );
    dev->clearS = stencil;
    }

  RenderState r0 = dev->renderState, rs = r0;
  rs.setColorMask(1,1,1,1);
  rs.setZWriting(1);
  setRenderState(d, rs);

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  setRenderState(d, r0);
  }

void Opengl2x::clear(AbstractAPI::Device *d, const Color &cl) const  {
  if( !setDevice(d) ) return;

  if( dev->clearColor!=cl ){
    glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
    dev->clearColor = cl;
    }

  RenderState r0 = dev->renderState, rs = r0;
  rs.setColorMask(1,1,1,1);
  setRenderState(d, rs);

  glClear( GL_COLOR_BUFFER_BIT );

  setRenderState(d, r0);
  }

void Opengl2x::clearZ(AbstractAPI::Device *d, float z ) const  {
  if( !setDevice(d) ) return;

  if( dev->clearDepth!=z ){
    dev->clearDepth = z;
  #ifdef __ANDROID__
    glClearDepthf( z );
  #else
    glClearDepth( z );
  #endif
    }

  RenderState r0 = dev->renderState, rs = r0;
  rs.setZWriting(1);
  setRenderState(d, rs);

  glClear( GL_DEPTH_BUFFER_BIT );

  setRenderState(d, r0);
  }

void Opengl2x::clearStencil( AbstractAPI::Device *d, unsigned s ) const  {
  if( !setDevice(d) ) return;

  if( dev->clearS!=s ){
    glClearStencil( s );
    dev->clearS = s;
    }
  glClear( GL_STENCIL_BUFFER_BIT );
  }

void Opengl2x::clear(AbstractAPI::Device *d,
                      float z, unsigned s ) const {
  if( !setDevice(d) ) return;

  if( dev->clearDepth!=z ){
    dev->clearDepth = z;
  #ifdef __ANDROID__
    glClearDepthf( z );
  #else
    glClearDepth( z );
  #endif
    }

  if( dev->clearS!=s ){
    glClearStencil( s );
    dev->clearS = s;
    }

  RenderState r0 = dev->renderState, rs = r0;
  rs.setZWriting(1);
  setRenderState(d, rs);

  glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  setRenderState(d, r0);
  }

void Opengl2x::clear(AbstractAPI::Device *d, const Color& cl, float z ) const{
  if( !setDevice(d) ) return;

  if( dev->clearColor!=cl ){
    glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
    dev->clearColor = cl;
    }

  if( dev->clearDepth!=z ){
    dev->clearDepth = z;
  #ifdef __ANDROID__
    glClearDepthf( z );
  #else
    glClearDepth( z );
  #endif
    }

  RenderState r0 = dev->renderState, rs = r0;
  rs.setColorMask(1,1,1,1);
  rs.setZWriting(1);
  setRenderState(d, rs);

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  setRenderState(d, r0);
  }

void Opengl2x::beginPaint( AbstractAPI::Device * d ) const {
  if( !setDevice(d) ) return;

  T_ASSERT( !dev->isPainting );

  dev->vbo    = 0;
  dev->ibo    = 0;
  dev->curVBO = 0;
  dev->curIBO = 0;
  dev->vbo    = 0;
  dev->ibo    = 0;
  dev->decl   = 0;

  dev->curVboOffsetIndex = 0;
  dev->curIboOffsetIndex = 0;

  dev->isPainting = true;

  setupBuffers( 0, true, true, false );
  T_ASSERT_X( errCk(), "OpenGL error" );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  T_ASSERT_X( errCk(), "OpenGL error" );
  T_ASSERT( setupFBO() );

  startTiledRender();
  }

void Opengl2x::endPaint  ( AbstractAPI::Device * d ) const{
  if( !setDevice(d) ) return;
  endTiledRender();

  T_ASSERT( dev->isPainting );
  dev->isPainting = false;
  setupBuffers( 0, false, true, false );
  dev->decl = 0;
  }

AbstractAPI::Texture *Opengl2x::createDepthStorage( AbstractAPI::Device *d,
                                                    int w, int h,
                                                    AbstractTexture::Format::Type f)
  const {
  if( !setDevice(d) ) return 0;
/*
  if( dev->potFboTegraBug && dev->npotFboTegraBug ){
    w = Device::nextPot(w);
    h = Device::nextPot(h);
    }*/

#ifdef __ANDROID__
  GLenum format[] = {
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT16
    };
#else
  GLenum format[] = {
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,
    GL_DEPTH_COMPONENT32
    };
#endif

  Detail::GLTexture * tex = dev->texPool.alloc();
  tex->w  = w;
  tex->h  = h;
  tex->id = 0;

  glGenRenderbuffers( 1, &tex->depthId );
  glBindRenderbuffer( GL_RENDERBUFFER, tex->depthId);
  glRenderbufferStorage( GL_RENDERBUFFER,
                       #ifndef __ANDROID__
                         //GL_DEPTH_STENCIL,
                         format[ f-AbstractTexture::Format::Depth16 ],
                       #else
                         GL_DEPTH_COMPONENT16,
                       #endif
                         w, h );
  glBindRenderbuffer( GL_RENDERBUFFER, 0);

  return (AbstractAPI::Texture*)tex;
  }

bool Opengl2x::setupFBO() const {
  const int maxMRT = dev->caps.maxRTCount;
    
  Detail::GLTexture* tex = 0;
  GLint w = 0, h = 0;
  for( int i=0; i<maxMRT; ++i ){
    if( dev->target.color[i] ){
      tex = dev->target.color[i];
      w = tex->w;
      h = tex->h;
      //fbo = &tex->fbo;
      break;
      }
    }

  int mrtSize = 0;
  for( int i=0; dev->target.color[i] && i<maxMRT; ++i )
    ++mrtSize;

  if( w==0 || h==0 || tex==0 ){
    return 1;
    }

  T_ASSERT_X( errCk(), "OpenGL error" );

  GLuint & fbo = dev->fbo.getTarget(dev->target, mrtSize, tex->fboHash);
  if( fbo==0 ){
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for( int i=0; i<maxMRT; ++i ){
      if( dev->target.color[i] ){
        // NVidia old driver bug issue

        bool potW = (w&(w-1))==0,
             potH = (h&(h-1))==0;
        if( !( potW && potH ) &&
            !( dev->target.color[i]->min == GL_NEAREST &&
               dev->target.color[i]->mag == GL_NEAREST &&
               dev->target.color[i]->clampU == GL_CLAMP_TO_EDGE &&
               dev->target.color[i]->clampV == GL_CLAMP_TO_EDGE ) ){
          glBindTexture( GL_TEXTURE_2D, dev->target.color[i]->id );
          if( dev->target.color[i]->min!=GL_NEAREST )
            glTexParameteri( GL_TEXTURE_2D,
                             GL_TEXTURE_MIN_FILTER,
                             GL_NEAREST );
          if( dev->target.color[i]->mag!=GL_NEAREST )
            glTexParameteri( GL_TEXTURE_2D,
                             GL_TEXTURE_MAG_FILTER,
                             GL_NEAREST );
          //*****
          if( dev->target.color[i]->clampU!=GL_CLAMP_TO_EDGE )
            glTexParameteri( GL_TEXTURE_2D,
                             GL_TEXTURE_WRAP_S,
                             GL_CLAMP_TO_EDGE );
          if( dev->target.color[i]->clampV!=GL_CLAMP_TO_EDGE )
            glTexParameteri( GL_TEXTURE_2D,
                             GL_TEXTURE_WRAP_T,
                             GL_CLAMP_TO_EDGE );

          dev->target.color[i]->min    = dev->target.color[i]->mag    = GL_NEAREST;
          dev->target.color[i]->clampU = dev->target.color[i]->clampV = GL_CLAMP_TO_EDGE;
          glBindTexture( GL_TEXTURE_2D, 0 );
          }
        dev->target.color[i]->mips = false;
        //*****

        glFramebufferTexture2D( GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0+i,
                                GL_TEXTURE_2D,
                                dev->target.color[i]->id,
                                dev->target.mip[i] );
        dev->target.color[i]->mips = false;
        }
      }

    if( dev->target.depth ){
      glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                                 GL_DEPTH_ATTACHMENT,
                                 GL_RENDERBUFFER,
                                 dev->target.depth->depthId );
      } else {
      glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                                 GL_DEPTH_ATTACHMENT,
                                 GL_RENDERBUFFER,
                                 0 );
      }
    } else {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

  T_ASSERT_X( errCk(), "OpenGL error" );

  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  if( !(status==0 || status==GL_FRAMEBUFFER_COMPLETE) ){
    struct Err{
      GLenum err;
      const char* desc;
    } err[] = {
    {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,         "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"},
    {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"},
    {GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,        "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"},
    {GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,        "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"},

    {GL_FRAMEBUFFER_UNSUPPORTED,                   "GL_FRAMEBUFFER_UNSUPPORTED"},
    {0,0}
    };
    const char* desc = "unknown error";
    for( int i=0; err[i].desc; ++i )
      if( err[i].err==status )
        desc = err[i].desc;
#ifndef __ANDROID__
    std::cout << "fbo error " << desc <<" " << std::hex << status << std::dec << std::endl;
#else
    void* ierr = (void*)status;
    __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "fbo error %s %p", desc, ierr);
#endif
    T_ASSERT_X(status==0, desc);
    return 1;
    }

#ifdef OGL_DEBUG
  T_ASSERT( status==0 || status==GL_FRAMEBUFFER_COMPLETE );
  T_ASSERT_X( errCk(), "OpenGL error" );
#endif

  glViewport( 0, 0, w, h );
  startTiledRender();

  T_ASSERT_X( errCk(), "OpenGL error" );
  return 1;
  }

void Opengl2x::startTiledRender() const {
  if( dev->isTileRenderStarted )
    return;

  if( dev->hasDiscardBuffers && (dev->target.color[1]==0) &&
      dev->target.color[0] ){
    GLenum flg[3] = {}, *p = flg;

    if( !dev->target.color[0] || !dev->target.color[0]->isInitalized ){
      *p = GL_COLOR_ATTACHMENT0; ++p;
      }
    if( !dev->target.depth || !dev->target.depth->isInitalized ){
      *p = GL_DEPTH_ATTACHMENT;  ++p;
      }

    dev->glDiscardFrameBuffer(GL_FRAMEBUFFER, p-flg, flg);
    errCk();
    } else
  if( dev->hasQCOMTiles ){
    int w = dev->scrW, h = dev->scrH;
    (void)w;
    (void)h;

    GLbitfield flg  = 0;
    GLbitfield nflg = GL_COLOR_BUFFER_BIT0_QCOM;

    for( int i=0; i<dev->caps.maxRTCount; ++i )
      if( dev->target.color[i] ){
        w = dev->target.color[i]->w;
        h = dev->target.color[i]->h;
        nflg = GL_NONE;

        if( dev->target.color[i]->isInitalized )
          flg |= GL_COLOR_BUFFER_BIT0_QCOM;
        }

    flg|=nflg;

    if( (dev->target.depth && dev->target.depth->isInitalized)||
        !dev->target.depth )
      flg |= GL_DEPTH_BUFFER_BIT0_QCOM;

    GLbitfield clr = 0;
    if( !(flg & GL_COLOR_BUFFER_BIT0_QCOM) )
      clr |= GL_COLOR_BUFFER_BIT;

    if( !(flg & GL_DEPTH_BUFFER_BIT) )
      clr |= (GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glClear( clr );

    //flg = GL_COLOR_BUFFER_BIT0_QCOM|GL_DEPTH_BUFFER_BIT0_QCOM|GL_STENCIL_BUFFER_BIT0_QCOM;
    //dev->isTileRenderStarted = true;
    //dev->glStartTilingQCOM( 0, 0, w, h, flg );
    }

  for( int i=0; i<dev->caps.maxRTCount; ++i )
    if( dev->target.color[i] ){
      dev->target.color[i]->isInitalized = true;
      }

  if( dev->target.depth )
    dev->target.depth->isInitalized = true;
  }

void Opengl2x::endTiledRender() const {
  if( dev->isTileRenderStarted && dev->hasDiscardBuffers ){
    if( dev->target.color[1]==0 &&  dev->target.color[0] ){
      GLenum flg[3] = {}, *p = flg;

      if( !dev->target.color[0] || !dev->target.color[0]->isInitalized ){
        *p = GL_COLOR_ATTACHMENT0; ++p;
        }
      if( !dev->target.depth || !dev->target.depth->isInitalized ){
        *p = GL_DEPTH_ATTACHMENT;  ++p;
        }

      dev->glDiscardFrameBuffer(GL_FRAMEBUFFER, p-flg, flg);
      errCk();
      }
    //dev->glDiscardFrameBuffer(GL_FRAMEBUFFER, 0,0);
    } else
  if( dev->isTileRenderStarted && dev->hasQCOMTiles ){
    GLbitfield flg  = 0;
    //flg = GL_COLOR_BUFFER_BIT0_QCOM|GL_DEPTH_BUFFER_BIT0_QCOM|GL_STENCIL_BUFFER_BIT0_QCOM;
    GLbitfield nflg = GL_COLOR_BUFFER_BIT0_QCOM;

    for( int i=0; i<dev->caps.maxRTCount; ++i )
      if( dev->target.color[i] )
        nflg = GL_NONE;

    flg|=nflg;

    if( nflg == GL_NONE ){
      for( int i=0; i<dev->caps.maxRTCount; ++i )
        if( dev->target.color[i] && dev->target.color[i]->isInitalized )
          flg |= GL_COLOR_BUFFER_BIT0_QCOM;
      }

    if( (dev->target.depth && dev->target.depth->isInitalized)||
        !dev->target.depth )
      flg |= GL_DEPTH_BUFFER_BIT0_QCOM;

    if(1){
      GLbitfield clr = 0;
      if( !(flg & GL_COLOR_BUFFER_BIT0_QCOM) )
        clr |= GL_COLOR_BUFFER_BIT;

      if( !(flg & GL_DEPTH_BUFFER_BIT) )
        clr |= (GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

      //glClear( clr );
      }

    dev->glEndTilingQCOM( flg );
    T_ASSERT_X( errCk(), "OpenGL error" );
    dev->isTileRenderStarted = false;
    }
  }

void Opengl2x::setRenderTaget( AbstractAPI::Device  *d,
                               AbstractAPI::Texture *t,
                               int mip,
                               int mrtSlot ) const {
  if( !setDevice(d) ) return;
  Detail::GLTexture *tex = (Detail::GLTexture*)t;
  dev->target.color[mrtSlot] = tex;
  dev->target.mip  [mrtSlot] = mip;
  }

void Opengl2x::unsetRenderTagets( AbstractAPI::Device *d,
                                  int /*count*/  ) const {
  if( !setDevice(d) ) return;
  endTiledRender();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  memset( &dev->target, 0, sizeof(dev->target) );

  glViewport( 0, 0, dev->scrW, dev->scrH );
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

AbstractAPI::StdDSSurface *Opengl2x::getDSSurfaceTaget( AbstractAPI::Device * ) const {
  return 0;
  }

void Opengl2x::retDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface * ) const {
  if( !setDevice(d) ) return;
  dev->target.depth = 0;
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface *tx ) const {
  if( !setDevice(d) ) return;
//#ifndef __ANDROID__
  dev->target.depth = (Detail::GLTexture*)tx;
//#endif
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::Texture *tx ) const {
  if( !setDevice(d) ) return;
//#ifndef __ANDROID__
  dev->target.depth = (Detail::GLTexture*)tx;
//#endif
  }

bool Opengl2x::startRender( AbstractAPI::Device *,bool   ) const {
#ifdef  __ANDROID__
  return AndroidAPI::startRender(0);
#endif
  return 1;
  }

bool Opengl2x::present(AbstractAPI::Device *d, SwapBehavior b) const {
  Device* dev = (Device*)d;
#ifndef __ANDROID__
  (void)b;
  SwapBuffers( dev->hDC );
#else
  static const EGLint se[2] = {EGL_BUFFER_PRESERVED, EGL_BUFFER_DESTROYED};

  if( dev->swapEfect!=se[b] ){
    dev->swapEfect = se[b];
    eglSurfaceAttrib( dev->disp, dev->s, EGL_SWAP_BEHAVIOR, dev->swapEfect );
    }

  eglSwapBuffers( dev->disp, dev->s );
  return AndroidAPI::present(0);
#endif
  return 0;
  }

bool Opengl2x::reset( AbstractAPI::Device *d,
                      void* hwnd,
                      const Options &opt ) const {
  if( !setDevice(d) ) return 0;

#ifndef __ANDROID__
  RECT rectWindow;
  GetClientRect( HWND(hwnd), &rectWindow);

  int w = rectWindow.right  - rectWindow.left;
  int h = rectWindow.bottom - rectWindow.top;

  if( opt.displaySettings.width>=0 )
    w = opt.displaySettings.width;

  if( opt.displaySettings.height>=0 )
    h = opt.displaySettings.height;

  dev->scrW = w;
  dev->scrH = h;

  AbstractAPI::setDisplaySettings( opt.displaySettings );
  glViewport(0,0, w,h);

  if( dev->wglSwapInterval )
    dev->wglSwapInterval( opt.vSync );
#else
  EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLSurface s    = eglGetCurrentSurface(EGL_DRAW);

  dev->disp = disp;
  dev->s    = s;

  EGLint w;
  EGLint h;
  eglQuerySurface( disp, s, EGL_WIDTH,  &w );
  eglQuerySurface( disp, s, EGL_HEIGHT, &h );
  
  dev->scrW = w;
  dev->scrH = h;
  glViewport(0,0,w,h);
#endif  

  dev->curIBO = 0;
  dev->curVBO = 0;
  dev->vbo    = 0;
  dev->ibo    = 0;
  return 1;
  }

bool Opengl2x::isFormatSupported(AbstractAPI::Device *d, Pixmap::Format f) const {
  Device* dev = (Device*)d;

  if( f==Pixmap::Format_RGB || f==Pixmap::Format_RGBA )
    return 1;

  if( dev->hasS3tcTextures && Pixmap::Format_DXT1 <=f && f<= Pixmap::Format_DXT5 )
    return 1;

  if( dev->hasETC1Textures && Pixmap::Format_ETC1_RGB8==f )
    return 1;

  return 0;
  }

AbstractAPI::Texture *Opengl2x::createTexture( AbstractAPI::Device *d,
                                               const Pixmap &p,
                                               bool  mips,
                                               bool  compress ) const {
  if( p.width()==0 || p.height()==0 )
    return 0;

  if( !setDevice(d) ) return 0;

  Detail::GLTexture* tex = dev->texPool.alloc();
  glGenTextures(1, &tex->id);
  glBindTexture(GL_TEXTURE_2D, tex->id);

  tex->mips     = mips;
  tex->compress = false;

  tex->w            = p.width();
  tex->h            = p.height();
  tex->isInitalized = true;

  static const GLenum frm[] = {
    GL_RGB,
    GL_RGBA,
    0x83F1,
    0x83F2,
    0x83F3,
    0x8D64//ETC1_RGB8_OES
    };

  tex->format = frm[p.format()];

  if( p.format()==Pixmap::Format_ETC1_RGB8 ){
    loadTextureETC(d, (AbstractAPI::Texture*)tex, p, mips, compress);
    } else
  if( p.format()==Pixmap::Format_RGB ||
      p.format()==Pixmap::Format_RGBA ){
    glTexImage2D( GL_TEXTURE_2D, 0, tex->format, p.width(), p.height(), 0,tex->format,
                  GL_UNSIGNED_BYTE, p.const_data() );
    if( mips )
      glGenerateMipmap( GL_TEXTURE_2D );
    } else {
    loadTextureS3TC(d, (AbstractAPI::Texture*)tex, p, mips, compress);
    }

  if( glGetError()==GL_INVALID_VALUE ){
    glDeleteTextures( 1, &tex->id );
    delete tex;
    return 0;
    }

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST );
  
  T_ASSERT_X( errCk(), "OpenGL error" );
  return ((AbstractAPI::Texture*)tex);
  }

void Opengl2x::loadTextureETC( AbstractAPI::Device *d,
                               Texture *t,
                               const Pixmap& p,
                               bool mips,
                               bool /*compress*/ ) const {
  setDevice(d);
  Detail::GLTexture *tex = (Detail::GLTexture*)t;

  const int nBlockSize = 8;

  glCompressedTexImage2D( GL_TEXTURE_2D, 0,
                          tex->format,//tex->format,
                          p.width(),
                          p.height(),
                          0,
                          size_t((p.width()/4)*(p.height()/4)*nBlockSize),
                          p.const_data() );
  T_ASSERT_X( errCk(), "OpenGL error" );

  tex->mips = false;
  if( mips && p.mipsCount()!=1 ){
    tex->mips = true;
    int mipCount = p.mipsCount();
#ifndef __ANDROID__
    glTexParameteri( GL_TEXTURE_2D,
                     0x813D,//GL_TEXTURE_MAX_LEVEL,
                     mipCount );
#endif
    T_ASSERT_X( errCk(), "OpenGL error" );

    int w = std::max(1, p.width() /2);
    int h = std::max(1, p.height()/2);
    const unsigned char * data =
        p.const_data() + ((p.width()+3)/4) * ((p.height()+3)/4) * nBlockSize;
    for( int i=1; i<=mipCount; ++i ){
      int nSize = ((w+3)/4) * ((h+3)/4) * nBlockSize;
      glCompressedTexImage2D( GL_TEXTURE_2D,
                              i,
                              tex->format,//tex->format,
                              w,
                              h,
                              0,
                              nSize,
                              data );
      data += nSize;
      if(w>1) w/=2;
      if(h>1) h/=2;
      T_ASSERT_X( errCk(), "OpenGL error" );
      }
    }
  }

void Opengl2x::loadTextureS3TC( AbstractAPI::Device  *d,
                                AbstractAPI::Texture* t,
                                const Pixmap& p,
                                bool mips,
                                bool /*compress*/ ) const{
  setDevice(d);
  Detail::GLTexture *tex = (Detail::GLTexture*)t;

  int nBlockSize = 16;
  if( p.format() == Pixmap::Format_DXT1 )
      nBlockSize = 8;

  int nSize = ((p.width()+3)/4) * ((p.height()+3)/4) * nBlockSize;
  glCompressedTexImage2D( GL_TEXTURE_2D, 0,
                          tex->format,//tex->format,
                          p.width(),
                          p.height(),
                          0,
                          nSize,
                          p.const_data() );
  if( mips ){
    int mipCount = p.mipsCount();//AbstractAPI::mipCount(p.width(),p.height());
    glTexParameteri( GL_TEXTURE_2D,
                     0x813D,//GL_TEXTURE_MAX_LEVEL,
                     mipCount );

    int w = std::max(1, p.width() /2);
    int h = std::max(1, p.height()/2);
    const unsigned char * data =
        p.const_data() + ((p.width()+3)/4) * ((p.height()+3)/4) * nBlockSize;
    for( int i=1; i<mipCount; ++i ){
      int nSize = ((w+3)/4) * ((h+3)/4) * nBlockSize;
      glCompressedTexImage2D( GL_TEXTURE_2D,
                              i,
                              tex->format,//tex->format,
                              w,
                              h,
                              0,
                              nSize,
                              data );
      data += nSize;
      if(w>1) w/=2;
      if(h>1) h/=2;
      T_ASSERT_X( errCk(), "OpenGL error" );
      }
    }
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

AbstractAPI::Texture *Opengl2x::recreateTexture( AbstractAPI::Device *d,
                                                 const Pixmap &p,
                                                 bool mips,
                                                 bool compress,
                                                 AbstractAPI::Texture *oldT ) const {
  if( oldT==0 )
    return createTexture(d, p, mips, compress);

  if( !setDevice(d) ) return 0;
  Detail::GLTexture* tex = 0;
  Detail::GLTexture* old = (Detail::GLTexture*)(oldT);

  static const GLenum frm[] = {
    GL_RGB,
    GL_RGBA,
    0x83F1,
    0x83F2,
    0x83F3,
    0x8D64//ETC1_RGB8_OES
    };

  GLenum format = frm[p.format()];

  if( int(old->w)      == p.width() &&
      int(old->h)      == p.height() &&
      old->format      == format &&
      old->mips        == mips   &&
      old->compress    == compress ){
    tex = old;
    } else {
    deleteTexture(d, oldT);
    return createTexture( d, p, mips, compress );
    }

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture(GL_TEXTURE_2D, tex->id);

  if( p.format()==Pixmap::Format_ETC1_RGB8 ){
    loadTextureETC(d, (AbstractAPI::Texture*)tex, p, mips, compress);
    } else
  if( p.format()==Pixmap::Format_RGB ||
      p.format()==Pixmap::Format_RGBA ){
#ifdef __ANDROID__
    glTexImage2D( GL_TEXTURE_2D, 0,
                  tex->format,
                  p.width(), p.height(),
                  0,tex->format,
                  GL_UNSIGNED_BYTE, p.const_data() );
#else
    glTexSubImage2D( GL_TEXTURE_2D, 0,
                     0, 0,
                     p.width(), p.height(),
                     tex->format,
                     GL_UNSIGNED_BYTE, p.const_data() );
#endif
    if( mips )
      glGenerateMipmap( GL_TEXTURE_2D );
    } else {
    loadTextureS3TC(d, (AbstractAPI::Texture*)tex, p, mips, compress);
    }
  
  return (AbstractAPI::Texture*)tex;
  }

AbstractAPI::Texture* Opengl2x::createTexture(AbstractAPI::Device *d,
                                               int w, int h,
                                               bool mips,
                                               AbstractTexture::Format::Type f,
                                               TextureUsage u  ) const {
  if( w==0 || h==0 )
    return 0;

  (void)u;
  if( !setDevice(d) ) return 0;

  if( f==AbstractTexture::Format::Depth16 ||
      f==AbstractTexture::Format::Depth24 ||
      f==AbstractTexture::Format::Depth32 ){
    return createDepthStorage(d,w,h,f);
    }

  GLenum storage, inFrm, bytePkg;
  dev->texFormat(f, storage, inFrm, bytePkg, u!=TU_Static );

  Detail::GLTexture* tex = dev->texPool.alloc();
  glGenTextures(1, &tex->id);
  glBindTexture(GL_TEXTURE_2D, tex->id);
  
  T_ASSERT_X( errCk(), "OpenGL error" );

  tex->w = w;
  tex->h = h;

  tex->format = storage;

  glTexImage2D( GL_TEXTURE_2D, 0,
                storage, w, h, 0,
                inFrm,
                bytePkg, 0 );

  T_ASSERT_X( errCk(), "OpenGL error" );

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST );

  if( mips!=0 )
    glGenerateMipmap(GL_TEXTURE_2D);
  
  T_ASSERT_X( errCk(), "OpenGL error" );
  return (AbstractAPI::Texture*)tex;
  }

void Opengl2x::generateMipmaps(AbstractAPI::Device *d, AbstractAPI::Texture *t) const {
  if( !setDevice(d) ) return;

  T_ASSERT_X( dev->isPainting==false, "cannot subdate texture while render in progress" );

  Detail::GLTexture* tex = (Detail::GLTexture*)t;

  GLenum texClass = GL_TEXTURE_2D;
  if( tex->z ){
#ifndef __ANDROID__
    texClass = GL_TEXTURE_3D;
#else
    return;
#endif
    }

  glBindTexture( texClass, tex->id );
  glGenerateMipmap( texClass );

  T_ASSERT_X( errCk(), "OpenGL error" );
  tex->mips = true;
  }

AbstractAPI::Texture *Opengl2x::createTexture3d(AbstractAPI::Device *d,
                                                 int x, int y, int z, bool mips,
                                                 AbstractTexture::Format::Type f,
                                                 TextureUsage u,
                                                 const char* data ) const {
  (void)d;
  (void)x;
  (void)y;
  (void)z;
  (void)mips;
  (void)f;
  (void)u;
  (void)data;

#ifdef __ANDROID__
  return 0;
#else
  if( x==0 || y==0 || z==0 )
    return 0;

  (void)u;
  if( !setDevice(d) ) return 0;

  GLenum storage, inFrm, bytePkg;
  dev->texFormat(f, storage, inFrm, bytePkg, u!=TU_Static );

  Detail::GLTexture* tex = dev->texPool.alloc();
  glGenTextures(1, &tex->id);
  glBindTexture(GL_TEXTURE_3D, tex->id);

  T_ASSERT_X( errCk(), "OpenGL error" );

  tex->w = x;
  tex->h = y;
  tex->z = z;

  tex->format = storage;

  glTexImage3D( GL_TEXTURE_3D, 0,
                storage, x, y, z, 0,
                inFrm,
                bytePkg,
                data );

  T_ASSERT_X( errCk(), "OpenGL error" );

  glTexParameteri( GL_TEXTURE_3D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_3D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST );

  if( mips!=0 )
    glGenerateMipmap(GL_TEXTURE_3D);

  T_ASSERT_X( errCk(), "OpenGL error" );
  return (AbstractAPI::Texture*)tex;
#endif
  }

void Opengl2x::deleteTexture( AbstractAPI::Device *d,
                              AbstractAPI::Texture *t ) const {
  if( !t )
    return;

  if( !setDevice(d) ) return;
  Detail::GLTexture* tex = (Detail::GLTexture*)t;

  dev->fbo.onDeleteTexture( tex );

  if( tex->id && glIsTexture(tex->id) )
    glDeleteTextures( 1, &tex->id );

  if( tex->depthId && glIsRenderbuffer(tex->depthId) )
    glDeleteRenderbuffers( 1, &tex->depthId );

  dev->texPool.free(tex);
  
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void Opengl2x::setTextureFlag( AbstractAPI::Device  *,
                               AbstractAPI::Texture *t,
                               TextureFlag f,
                               bool v ) const {
  Detail::GLTexture* tex = (Detail::GLTexture*)t;

  if( f==TF_Inialized )
    tex->isInitalized = v;
  }

AbstractAPI::VertexBuffer*
     Opengl2x::createVertexBuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize,
                                   BufferUsage u  ) const{
  return createVertexBuffer(d, size, elSize, 0, u );
  }

AbstractAPI::VertexBuffer *Opengl2x::createVertexBuffer( AbstractAPI::Device *d,
                                                         size_t size,
                                                         size_t elSize,
                                                         const void *src,
                                                         BufferUsage u ) const {
  if( !setDevice(d) ) return 0;

  static const GLenum gu[] = {
    GL_STREAM_DRAW,
    GL_DYNAMIC_DRAW,
    GL_STATIC_DRAW
    };

  Detail::GLBuffer *vbo = dev->bufPool.alloc();
  vbo->offset = 0;
  vbo->size   = 0;

  glGenBuffers( 1, &vbo->id );
  glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  glBufferData( GL_ARRAY_BUFFER,
                size*elSize, src,
                gu[u] );
  T_ASSERT_X( errCk(), "OpenGL error" );

  return (AbstractAPI::VertexBuffer*)vbo;
  }

void Opengl2x::deleteVertexBuffer(  AbstractAPI::Device *d,
                                    AbstractAPI::VertexBuffer*v ) const{
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;
  if( glIsBuffer(vbo->id) ){
    if( dev->curVBO==vbo->id ){
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      dev->curVBO = 0;
      }

    glDeleteBuffers( 1, &vbo->id );
    } else {
    T_ASSERT_X(0, "bad glDeleteBuffers call");
    }
  dev->bufPool.free(vbo);

  T_ASSERT_X( errCk(), "OpenGL error" );
  }

AbstractAPI::IndexBuffer*
     Opengl2x::createIndexBuffer( AbstractAPI::Device *d,
                                  size_t size, size_t elSize,
                                  BufferUsage u ) const {
  return createIndexBuffer(d, size, elSize, 0, u);
  }

AbstractAPI::IndexBuffer *Opengl2x::createIndexBuffer( AbstractAPI::Device *d,
                                                       size_t size,
                                                       size_t elSize,
                                                       const void *src,
                                                       BufferUsage u  ) const {
  if( !setDevice(d) ) return 0;

  static const GLenum gu[] = {
    GL_STREAM_DRAW,
    GL_DYNAMIC_DRAW,
    GL_STATIC_DRAW
    };

  Detail::GLBuffer *ibo = dev->bufPool.alloc();
  ibo->offset = 0;
  ibo->size   = 0;

  glGenBuffers( 1, &ibo->id );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo->id );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                size*elSize, src,
                gu[u] );
  T_ASSERT_X( errCk(), "OpenGL error" );

  return (AbstractAPI::IndexBuffer*)ibo;
  }

void Opengl2x::deleteIndexBuffer( AbstractAPI::Device *d,
                                  AbstractAPI::IndexBuffer* v) const {
  if( !setDevice(d) ) return;

  Detail::GLBuffer *ibo = (Detail::GLBuffer*)v;
  if( glIsBuffer(ibo->id) ){
    if( dev->curIBO==ibo->id ){
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      dev->curIBO = 0;
      }

    glDeleteBuffers( 1, &ibo->id );
    }
  dev->bufPool.free(ibo);

  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void* Opengl2x::lockBuffer( AbstractAPI::Device *d,
                            AbstractAPI::VertexBuffer * v,
                            unsigned offset,
                            unsigned size ) const {
  if( !setDevice(d) ) return 0;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  //vbo->mappedData = (char*)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );

  if( !dev->lbUseed ){
    dev->lbUseed = true;
    dev->tmpLockBuffer.resize( size );
    vbo->mappedData = &dev->tmpLockBuffer[0];
    } else {
    vbo->mappedData = new char[size];
    }

  vbo->offset = offset;
  vbo->size   = size;

  T_ASSERT_X( errCk(), "OpenGL error" );
  return vbo->mappedData;
  }

void Opengl2x::unlockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::VertexBuffer* v) const {  
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ARRAY_BUFFER, vbo->offset, vbo->size, vbo->mappedData );

  if( vbo->mappedData == &dev->tmpLockBuffer[0] ){
    dev->lbUseed = false;
    dev->tmpLockBuffer.clear();
    } else {
    delete[] vbo->mappedData;
    }
  vbo->mappedData = 0;

  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  //glUnmapBuffer( GL_ARRAY_BUFFER );
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void* Opengl2x::lockBuffer( AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer * v,
                            unsigned offset, unsigned size) const {
  (void)size;

  if( !setDevice(d) ) return 0;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  if( !dev->lbUseed ){
    dev->lbUseed = true;
    dev->tmpLockBuffer.resize( size );
    vbo->mappedData = &dev->tmpLockBuffer[0];
    } else {
    vbo->mappedData = new char[size];
    }

  vbo->offset = offset;
  vbo->size   = size;

  T_ASSERT_X( errCk(), "OpenGL error" );
  return vbo->mappedData;
  }

void Opengl2x::unlockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::IndexBuffer* v) const {
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, vbo->offset, vbo->size, vbo->mappedData );

  if( vbo->mappedData == &dev->tmpLockBuffer[0] ){
    dev->lbUseed = false;
    dev->tmpLockBuffer.clear();
    } else {
    delete[] vbo->mappedData;
    }
  vbo->mappedData = 0;

  //glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, *vbo );
  //glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

const AbstractShadingLang*
        Opengl2x::createShadingLang( AbstractAPI::Device *d ) const {
  if( !setDevice(d) ) return 0;
  AbstractAPI::OpenGL2xDevice *dev =
      reinterpret_cast<AbstractAPI::OpenGL2xDevice*>(d);

  return new Tempest::GLSL( dev );
  }

void Opengl2x::deleteShadingLang( const AbstractShadingLang * l ) const {
  delete l;
  }

AbstractAPI::VertexDecl*
      Opengl2x::createVertexDecl( AbstractAPI::Device *d,
                                  const VertexDeclaration::Declarator &de ) const {
  Device* dev = (Device*)d;
  VertexDeclaration::Declarator *dx = dev->declPool.alloc(de);
  return (AbstractAPI::VertexDecl*)dx;
  }

void Opengl2x::deleteVertexDecl( AbstractAPI::Device *d,
                                 AbstractAPI::VertexDecl* de ) const {
  Device* dev = (Device*)d;

  VertexDeclaration::Declarator *dx = (VertexDeclaration::Declarator*)(de);
  if( dev->decl==dx )
    dev->decl = 0;

  dev->declPool.free(dx);
  }

void Opengl2x::setVertexDeclaration( AbstractAPI::Device *d,
                                     AbstractAPI::VertexDecl* de ) const {
  if( !setDevice(d) ) return;

  if( dev->decl==(VertexDeclaration::Declarator*)de )
    return;

  dev->decl   = (VertexDeclaration::Declarator*)de;

  if( dev->isPainting )
    setupBuffers( 0, true, true, false );
  }

void Opengl2x::bindVertexBuffer( AbstractAPI::Device *d,
                                 AbstractAPI::VertexBuffer* b,
                                 int vsize ) const {
  if( !setDevice(d) ) return;

  dev->vbo        = *(GLuint*)b;
  dev->vertexSize = vsize;
  }

void Opengl2x::bindIndexBuffer( AbstractAPI::Device * d,
                                AbstractAPI::IndexBuffer * b ) const {
  if( !setDevice(d) ) return;

  dev->ibo = *(GLuint*)b;
  }

void Opengl2x::setupBuffers( int vboOffsetIndex,
                             bool on,
                             bool enable,
                             bool bind ) const {
  if( !dev->decl )
    return;

  bool rebind = !(dev->curVBO == dev->vbo && dev->curVboOffsetIndex==vboOffsetIndex );

  if( bind && dev->vbo && rebind ){
    dev->curVBO            = dev->vbo;
    dev->curVboOffsetIndex = vboOffsetIndex;

    T_ASSERT( glIsBuffer(dev->vbo) );
    glBindBuffer( GL_ARRAY_BUFFER, dev->vbo );
    }

  static const GLenum vfrm[] = {
    GL_FLOAT, //double0 = 0, // just trick
    GL_FLOAT, //double1 = 1, GL_HALF_FLOAT_OES
    GL_FLOAT, //double2 = 2,
    GL_FLOAT, //double3 = 3,
    GL_FLOAT, //double4 = 4,

    GL_UNSIGNED_BYTE, //color  = 5,
    GL_SHORT,//short2 = 6,
    GL_SHORT,//short4 = 7

    GL_HALF_FLOAT,//half2 = 8,
    GL_HALF_FLOAT,//half4 = 9
    };

  static const int counts[] = {
    0, 1, 2, 3, 4, 4, 2, 4, 2, 4
    };

  static const int strides[] = {
    0,  4*1, 4*2, 4*3, 4*4,  4, 2*2, 2*4, 2*2, 2*4
    };

  size_t stride = vboOffsetIndex;

  const Tempest::VertexDeclaration::Declarator& vd
      = *(const Tempest::VertexDeclaration::Declarator*)dev->decl;
  for( int i=0; i<vd.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e = vd[i];

    int usrAttr = -1;
    int loc = i;
    if( vd[i].usage==Usage::TexCoord ){
      loc = vd.size()+vd[i].index;
      } else
    if( vd[i].attrName.size() ){
      loc = vd.size()+vd.texCoordCount()+usrAttr;
      ++usrAttr;
      }

    int count  = counts[e.component];
    GLenum frm =   vfrm[e.component];

    if( enable ){
      if( int(dev->vAttrLoc.size()) <= loc )
        dev->vAttrLoc.resize(loc+1, false);

      if( dev->vAttrLoc[loc]!=on ){
        dev->vAttrLoc[loc] = on;
        if( on ){
          glEnableVertexAttribArray(loc);
          } else {
          glDisableVertexAttribArray(loc);
          }
        }
      }

    if( bind && dev->vbo && rebind ){
      if( e.component==Decl::color )
        glVertexAttribPointer( loc, count, frm, GL_TRUE,
                               dev->vertexSize, (void*)stride );
      else
        glVertexAttribPointer( loc, count, frm, GL_FALSE,
                               dev->vertexSize, (void*)stride );
      }

    stride += strides[e.component];
    }

  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void Opengl2x::draw( AbstractAPI::Device *de,
                     AbstractAPI::PrimitiveType t,
                     int firstVertex, int pCount ) const {
  setDevice(de);

  setupBuffers( 0, false, false, true );

  if( dev->curIBO!=0 ){
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    dev->curIBO = 0;
    }

  static const GLenum type[] = {
    GL_POINTS,         // = 1,
    GL_LINES,          // = 2,
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = vertexCount(t, pCount);
  glDrawArrays( type[ t-1 ], firstVertex, vpCount );
  //setupBuffers(0, false, false);
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

//unstable
void Opengl2x::drawIndexed( AbstractAPI::Device *de,
                            AbstractAPI::PrimitiveType t,
                            int vboOffsetIndex,
                            int iboOffsetIndex,
                            int pCount ) const {
  setDevice(de);

  if( !dev->ibo )
    return;

  bool rebind = !(dev->curIBO == dev->ibo && dev->curIboOffsetIndex==iboOffsetIndex );
  setupBuffers( vboOffsetIndex*dev->vertexSize, false, false, true );

  if( rebind ){
    dev->curIBO            = dev->ibo;
    dev->curIboOffsetIndex = iboOffsetIndex;
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, dev->ibo );
    }
  T_ASSERT_X( errCk(), "OpenGL error" );

  static const GLenum type[] = {
    GL_POINTS,         // = 1,
    GL_LINES,          // = 2,
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = vertexCount(t, pCount);
  glDrawElements( type[ t-1 ],
                  vpCount,
                  GL_UNSIGNED_SHORT,
                  ((void*)(iboOffsetIndex*sizeof(uint16_t))) );

  T_ASSERT_X( errCk(), "OpenGL error" );
  }

bool Opengl2x::hasManagedStorge() const {
#ifdef __ANDROID__
  //return false;
#endif
  return true;
  }

Size Opengl2x::windowSize( Tempest::AbstractAPI::Device * d ) const {
  Device *dev = (Device*)d;
  return Size(dev->scrW, dev->scrH);
  }

void Opengl2x::setRenderState( AbstractAPI::Device *d,
                               const RenderState & r) const {
  if( !setDevice(d) ) return;

  GLenum cull[] = {
    GL_NONE,
    GL_FRONT,
    GL_BACK
    };

  GLenum cmp[] = {
    GL_NEVER,

    GL_GREATER,
    GL_LESS,

    GL_GEQUAL,
    GL_LEQUAL,

    GL_NOTEQUAL,
    GL_EQUAL,
    GL_ALWAYS,
    GL_ALWAYS
    };

  if( dev->renderState.cullFaceMode()!=r.cullFaceMode() )
    glCullFace( cull[ r.cullFaceMode() ] );

  /*
  if( !r.isZTest() && !r.isZWriting() )
    glDisable(GL_DEPTH_TEST); else
    glEnable(GL_DEPTH_TEST);
  */
  
#ifdef __ANDROID__
  //a-test not supported
  //glEnable( GL_BLEND );
#else
  if( r.alphaTestMode()!=Tempest::RenderState::AlphaTestMode::Always ){
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( cmp[ r.alphaTestMode() ], r.alphaTestRef() );
    } else {
    glDisable( GL_ALPHA_TEST );
    }
#endif

  GLenum  blend[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA_SATURATE,
    GL_ZERO
    };

#ifndef __ANDROID__
  GLenum rmode[] = {
    GL_FILL,
    GL_LINE,
    GL_POINT
    };

  glPolygonMode( GL_FRONT, rmode[r.frontPolygonRenderMode()] );
  glPolygonMode( GL_BACK,  rmode[r.backPolygonRenderMode()] );
#endif

  if( dev->isWriteOnly(r) ){
    glColorMask(1,1,1,1);
    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glDepthMask(0);
    dev->enableWriteOnlyRender( 1 );
    } else {
    dev->enableWriteOnlyRender( 0 );

    bool w[4], w1[4];
    r.getColorMask( w[0], w[1], w[2], w[3] );
    dev->renderState.getColorMask( w1[0], w1[1], w1[2], w1[3] );

    if( w[0]!=w1[0] ||
        w[1]!=w1[1] ||
        w[2]!=w1[2] ||
        w[3]!=w1[3] ){
      glColorMask( w[0], w[1], w[2], w[3] );
      }

    if( dev->renderState.isBlend()!=r.isBlend() ||
        dev->renderState.getBlendDFactor()!=r.getBlendDFactor() ||
        dev->renderState.getBlendSFactor()!=r.getBlendSFactor()  ){
      if( r.isBlend() ){
        glBlendFunc( blend[ r.getBlendSFactor() ], blend[ r.getBlendDFactor() ] );
        glEnable( GL_BLEND );
        } else {
        glDisable( GL_BLEND );
        }
      }

    glEnable( GL_DEPTH_TEST );
    if( dev->renderState.isZTest() != r.isZTest() ||
        dev->renderState.getZTestMode()!=r.getZTestMode() ){
      if( r.isZTest() ){
        glDepthFunc( cmp[ r.getZTestMode() ] );
        } else {
        glDisable( GL_DEPTH_TEST );
        }
      }

    if( dev->renderState.isZWriting()!=r.isZWriting() )
      glDepthMask( r.isZWriting() );
    }

  dev->renderState = r;

  T_ASSERT_X( errCk(), "OpenGL error" );
  }
