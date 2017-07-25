#include "opengl2x.h"

#include <Tempest/Platform>

#ifdef __LINUX__
#include "system/linuxapi.h"
#endif

#ifdef __MOBILE_PLATFORM__
//GL_HALF_FLOAT -> GL_HALF_FLOAT_OES -> 0x8D61
#define GL_HALF_FLOAT 0x8D61
#endif

#ifdef __ANDROID__

//#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#else
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include "glfn.h"

#ifdef __APPLE__
#ifdef __MOBILE_PLATFORM__
#include <OpenGLES/ES2/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#include "system/osxapi.h"
#include "system/iosapi.h"
#else
#include <GL/gl.h>
#endif

using namespace Tempest::GLProc;
#endif

#include <Tempest/GLSL>
#include <Tempest/RenderState>
#include <Tempest/Log>
#include <Tempest/Pixmap>
#include <Tempest/Assert>

#include <squish.h>
#include <cstring>
#include <algorithm>
#include <thread>

#include "gltypes.h"

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#endif

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

#define GL_PATCHES 0x0000000e
//#define GL_MAX_VARYING_VECTORS                        0x8DFC

#ifdef __ANDROID__
static const GLubyte* vstr( const GLubyte* v ){
  if( v )
    return v; else
    return (GLubyte*)""; //avoid Android bug
  }
#endif

using namespace Tempest;

struct Tempest::Opengl2x::ImplDevice : public Detail::ImplDeviceBase {
  ImplDevice(){
#ifdef __ANDROID__
    AndroidAPI::onSurfaceDestroyed.bind(this,&ImplDevice::surfaceDestroyed);
#endif
    }
  ~ImplDevice(){
#ifdef __ANDROID__
    AndroidAPI::onSurfaceDestroyed.ubind(this,&ImplDevice::surfaceDestroyed);
#endif
    }

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

#ifdef __ANDROID__
  void surfaceDestroyed(){
    if(surface==EGL_NO_SURFACE)
      return;
    eglMakeCurrent(disp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    eglDestroySurface(disp,surface);
    surface=EGL_NO_SURFACE;
    }

  void ajustSurface(Jni::AndroidWindow& window){
    if(surface!=EGL_NO_SURFACE)
      return;
    eglMakeCurrent(disp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    surface = eglCreateWindowSurface( disp, config, window, NULL);
    }
#endif
  };

Opengl2x::Opengl2x():dev(0){
  }

Opengl2x::~Opengl2x(){
  }

Opengl2x::Caps Opengl2x::caps( AbstractAPI::Device* d ) const {
  ImplDevice * dev = (ImplDevice*)d;
  return dev->caps;
  }

GraphicsSubsystem::Device* Opengl2x::createDevice(void *hwnd, const AbstractAPI::Options &opt) const {
#ifdef __WINDOWS__
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

  pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion   = 1;
  pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;

  HDC    hDC        = GetDC( HWND(hwnd) );
  GLint pixelFormat = ChoosePixelFormat( hDC, &pfd );
  SetPixelFormat( hDC, pixelFormat, &pfd);
#endif
  return AbstractAPI::createDevice(hwnd,opt);
  }

void Opengl2x::deleteDevice(GraphicsSubsystem::Device *d) const {
  return AbstractAPI::deleteDevice(d);
  }

bool Opengl2x::createContext( Detail::ImplDeviceBase* dev,
                              void *hwnd, const Options& /*opt*/) const {
#ifdef __APPLE__
  dev->window  = hwnd;
#ifdef __MOBILE_PLATFORM__
  dev->context = iOSAPI::initializeOpengl(hwnd);
  return dev->context!=nullptr && iOSAPI::glMakeCurrent(dev->context);
#else
  dev->context = OsxAPI::initializeOpengl(hwnd);
  return dev->context!=nullptr && OsxAPI::glMakeCurrent(dev->context);
#endif
#endif

#ifdef __WINDOWS__
  HDC   hDC = GetDC( HWND(hwnd) );
  HGLRC hRC = Detail::createContext( hDC );
  wglMakeCurrent( hDC, hRC );

  dev->hRC = hRC;

  if( !Detail::initGLProc() ) {
    return 0;
    }
  return 1;
#endif

#ifdef __ANDROID__
  static const EGLint attribs16[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_BLUE_SIZE,    5,
    EGL_GREEN_SIZE,   6,
    EGL_RED_SIZE,     5,
    EGL_ALPHA_SIZE,   0,
    EGL_DEPTH_SIZE,   16,
    EGL_STENCIL_SIZE, 0,
    EGL_NONE
    };

  static const EGLint attribs4[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_BLUE_SIZE,    4,
    EGL_GREEN_SIZE,   4,
    EGL_RED_SIZE,     4,
    EGL_NONE
    };

  const EGLint* attribs = attribs16;

  EGLint             w=0, h=0, format=0;
  EGLint             numConfigs=0;
  EGLConfig          ini_config =nullptr;
  EGLSurface         ini_surface=nullptr;
  EGLContext         ini_context=nullptr;
  Jni::AndroidWindow ini_window;
  EGLDisplay         ini_display=nullptr;

  ini_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  ini_window  = AndroidAPI::nWindow(hwnd);

  if(!eglInitialize(ini_display, 0, 0))
    Log::e("eglInitialize failed");

  if(!eglChooseConfig(ini_display, attribs, &ini_config, 1, &numConfigs))
    Log::e("eglChooseConfig failed"); else
    Log::e("eglChooseConfig = ",ini_config);

  if(numConfigs==0)
    eglChooseConfig(ini_display, attribs4, &ini_config, 1, &numConfigs);

  Log::d("numConfigs=",numConfigs);
  eglGetConfigAttrib(ini_display, ini_config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry( ini_window, 0, 0, format);
  ini_surface = eglCreateWindowSurface( ini_display, ini_config, ini_window, NULL);

  const EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  ini_context = eglCreateContext(ini_display, ini_config, EGL_NO_CONTEXT, attrib_list);
  if( ini_context==EGL_NO_CONTEXT )
    Log::e("Unable to create context");

  Log::d("Surface=", ini_surface, " Display=", ini_display, " Context=", ini_context);
  if( eglMakeCurrent(ini_display, ini_surface, ini_surface, ini_context) == EGL_FALSE ){
    Log::e("Unable to eglMakeCurrent");
    return -1;
    }

  eglQuerySurface(ini_display, ini_surface, EGL_WIDTH,  &w);
  eglQuerySurface(ini_display, ini_surface, EGL_HEIGHT, &h);

  dev->disp    = ini_display;
  dev->context = ini_context;
  dev->surface = ini_surface;
  dev->config  = ini_config;

  dev->scrW = w;
  dev->scrH = h;

  /*
  if( wnd && display != EGL_NO_DISPLAY ){
    forceToResize = true;
    }
  */
  dev->swapEfect = EGL_BUFFER_PRESERVED;
  return 1;
#endif

#ifdef __LINUX__
  dev->dpy        = (Display*)LinuxAPI::display();
  Display *dpy    = dev->dpy;//FIXME
  GLint att[]     = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo *vi = glXChooseVisual (dpy, 0, att);
  dev->glc        = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  XFree( vi );

  glXMakeCurrent( dev->dpy, dev->window, dev->glc);

  if( !Detail::initGLProc() ) {
    return 0;
    }
  return 1;
#endif
  return 0;
  }

AbstractAPI::Device* Opengl2x::allocDevice(void *hwnd, const Options &opt) const {
  ImplDevice* dev = new ImplDevice();

  if( !createContext(dev, hwnd, opt) )
    return 0;

#ifdef __LINUX__
  dev->window = *((::Window*)hwnd);
#endif

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "vendor = %s",     vstr(glGetString(GL_VENDOR))     );
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "render = %s",     vstr(glGetString(GL_RENDERER))   );
  __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "extensions = %s", vstr(glGetString(GL_EXTENSIONS)) );
#endif

  dev->initExt();
  T_ASSERT_X( errCk(), "OpenGL error" );

  glEnable( GL_DEPTH_TEST );
  glFrontFace( GL_CW );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  reset( (AbstractAPI::Device*)dev, hwnd, opt );
  setRenderState( (AbstractAPI::Device*)dev, Tempest::RenderState() );

  T_ASSERT_X( errCk(), "OpenGL error" );
  return (AbstractAPI::Device*)dev;
  }

void Opengl2x::freeDevice(AbstractAPI::Device *d) const {
  ImplDevice* dev = (ImplDevice*)d;
  if( !setDevice(d) ) return;
  dev->free(dev->dynVbo);

#ifdef __WINDOWS__
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( dev->hRC );
#endif

#ifdef __LINUX__
  glXDestroyContext( dev->dpy, dev->glc );
  glXMakeCurrent( dev->dpy, None, NULL);
#endif

#ifdef __ANDROID__
  eglMakeCurrent(dev->disp,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
  eglDestroyContext(dev->disp,dev->context);
  eglDestroySurface(dev->disp,dev->surface);
#endif

  delete dev;
  }

bool Opengl2x::setDevice( AbstractAPI::Device * d ) const {
  dev = (ImplDevice*)d;
  T_ASSERT_X( errCk(), "OpenGL error" );
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
    setClearDepth(z);
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
    setClearDepth(z);
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
    setClearDepth(z);
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
  #ifdef __MOBILE_PLATFORM__
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

#ifdef __MOBILE_PLATFORM__
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
                         format[ f-AbstractTexture::Format::Depth16 ],
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
        }
      }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for( int i=0; i<mrtSize; ++i )
      if( dev->target.color[i] ){
        glFramebufferTexture2D( GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0+i,
                                GL_TEXTURE_2D,
                                dev->target.color[i]->id,
                                dev->target.mip[i] );
        dev->target.color[i]->mips = false;
        }

    if( dev->target.depth ){
      if( dev->target.depth->id )
        glFramebufferTexture2D( GL_FRAMEBUFFER,
                                GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D,
                                dev->target.depth->id, 0 );
        else
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

#ifndef __MOBILE_PLATFORM__
    {
      GLenum buf[32];
      for( int i=0; i<mrtSize && i<32; ++i )
        buf[i] = GL_COLOR_ATTACHMENT0+i;
      glDrawBuffers(mrtSize, buf);
    }
#endif
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
    printf("OpenGL : fbo error %s 0x%X", desc, int(status));
#else
    __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "fbo error %s 0x%X", desc, int(status));
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

#ifdef __IOS__
  iOSAPI::glBindZeroFramebuffer(dev->window);
#else
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
  /*
  glReadBuffer(GL_BACK);
  glDrawBuffer(GL_BACK);
  */

  memset( &dev->target, 0, sizeof(dev->target) );

  glViewport( 0, 0, dev->scrW, dev->scrH );
  T_ASSERT_X( errCk(), "OpenGL error" );
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

    if(p-flg>0)
      dev->glDiscardFrameBuffer(GL_FRAMEBUFFER, GLsizei(p-flg), flg);
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

      if( dev->target.color[0] && !dev->target.color[0]->isInitalized ){
        *p = GL_COLOR_ATTACHMENT0; ++p;
        }
      if( dev->target.depth && !dev->target.depth->isInitalized ){
        //*p = GL_DEPTH_ATTACHMENT;  ++p;
        }

      if(p-flg>0)
        dev->glDiscardFrameBuffer(GL_FRAMEBUFFER, GLsizei(p-flg), flg);
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

bool Opengl2x::startRender(AbstractAPI::Device* d,void *hwnd, bool   ) const {
  if( !setDevice(d) ) return false;
#ifdef  __ANDROID__
  ImplDevice& degl=*((ImplDevice*)dev);

  if(degl.surface==EGL_NO_SURFACE) {
    Jni::AndroidWindow window = AndroidAPI::nWindow(hwnd);
    degl.ajustSurface(window);
    }
  return eglMakeCurrent(dev->disp,dev->surface,dev->surface,dev->context)==EGL_TRUE;
#endif
#ifdef __LINUX__
  glXMakeCurrent( dev->dpy, dev->window, dev->glc);
#endif
#ifdef __WINDOWS__
  HDC hDC = GetDC( HWND(hwnd) );
  if(!wglMakeCurrent( hDC, dev->hRC )){
    Log::e("wglMakeCurrent failed: ",uint32_t(GetLastError()));
    T_ASSERT(0);
    }
#endif
#ifdef __APPLE__
#ifdef __MOBILE_PLATFORM__
  return iOSAPI::glMakeCurrent(dev->context);
#else
  return OsxAPI::glMakeCurrent(dev->context);
#endif
#endif
  setupViewport(d,hwnd);
  return 1;
  }

bool Opengl2x::present(AbstractAPI::Device *d, void *hwnd, SwapBehavior b) const {
  ImplDevice* dev = (ImplDevice*)d;
  (void)dev;
  (void)hwnd;
#ifdef __WINDOWS__
  (void)b;
  HDC hDC = GetDC( HWND(hwnd) );
  SwapBuffers( hDC );
#endif

#ifdef __LINUX__
  (void)b;
  glXSwapBuffers( dev->dpy, dev->window );
#endif

#ifdef __ANDROID__
  static const EGLint se[2] = {EGL_BUFFER_PRESERVED, EGL_BUFFER_DESTROYED};

  if( dev->swapEfect!=se[b] ){
    dev->swapEfect = se[b];
    eglSurfaceAttrib( dev->disp, dev->surface, EGL_SWAP_BEHAVIOR, dev->swapEfect );
    }

  eglSwapBuffers( dev->disp, dev->surface );
  return AndroidAPI::present(0);
#endif
#ifdef __APPLE__
  (void)b;
#ifdef __MOBILE_PLATFORM__
  iOSAPI::glSwapBuffers(dev->window,dev->context);
#else
  OsxAPI::glSwapBuffers(dev->context);
#endif
#endif
  return 0;
  }

bool Opengl2x::reset( AbstractAPI::Device *d,
                      void* hwnd,
                      const Options &opt ) const {
  if( !setDevice(d) ) return 0;
  dev->displaySettings = opt.displaySettings;

#ifdef __APPLE__
  Size sz   = SystemAPI::instance().windowClientRect((SystemAPI::Window*)hwnd);
  dev->scrW = sz.w;
  dev->scrH = sz.h;

  AbstractAPI::setDisplaySettings( hwnd, opt.displaySettings );

#ifdef __MOBILE_PLATFORM__
  iOSAPI::glMakeCurrent( dev->context );
  iOSAPI::glUpdateContext(dev->context,dev->window);
  glViewport(0,0, dev->scrW, dev->scrH);
#else
  OsxAPI::glMakeCurrent( dev->context );
  OsxAPI::glUpdateContext(dev->context,dev->window);
  glViewport(0,0, dev->scrW, dev->scrH);
#endif
#endif

#ifdef __LINUX__
  ::Window* w = (::Window*)hwnd;

  ::Window root;
  int x, y;
  unsigned ww, hh, border, depth;

  XGetGeometry(dev->dpy, *w, &root, &x, &y, &ww, &hh, &border, &depth);

  dev->scrW = ww;
  dev->scrH = hh;

  AbstractAPI::setDisplaySettings( hwnd, opt.displaySettings );

  glXMakeCurrent( dev->dpy, dev->window, dev->glc);
  glViewport(0,0, dev->scrW, dev->scrH);
#endif

#ifdef __WINDOWS__
  setupViewport(d,hwnd);
  if( dev->displaySettings!=DisplaySettings(-1,-1) )
    AbstractAPI::setDisplaySettings( hwnd, opt.displaySettings );

  if( dev->wglSwapInterval )
    dev->wglSwapInterval( opt.vSync );
#endif

#ifdef __ANDROID__
  (void)opt;

  ImplDevice& degl=*((ImplDevice*)dev);

  EGLDisplay         disp   = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  Jni::AndroidWindow window = AndroidAPI::nWindow(hwnd);

  dev->disp = disp;
  degl.ajustSurface(window);

  EGLint w=ANativeWindow_getWidth (window);
  EGLint h=ANativeWindow_getHeight(window);
  
  dev->scrW = w;
  dev->scrH = h;

  eglMakeCurrent(disp,dev->surface,dev->surface,dev->context);
  glViewport(0,0,w,h);
#endif  

  dev->curIBO = 0;
  dev->curVBO = 0;
  dev->vbo    = 0;
  dev->ibo    = 0;
  return 1;
  }

void Opengl2x::setupViewport(AbstractAPI::Device *d,void *hwnd) const {
  if( !setDevice(d) ) return;
  (void)hwnd;

#ifdef __WINDOWS__
  RECT rectWindow;
  GetClientRect( HWND(hwnd), &rectWindow);

  int w = rectWindow.right  - rectWindow.left;
  int h = rectWindow.bottom - rectWindow.top;

  if( dev->displaySettings.width>=0 && dev->displaySettings.height>=0 ) {
    w = dev->displaySettings.width;
    h = dev->displaySettings.height;
    }

  dev->scrW = w;
  dev->scrH = h;

  glViewport(0,0, w,h);
#endif
  }

bool Opengl2x::isFormatSupported(AbstractAPI::Device *d, Pixmap::Format f) const {
  ImplDevice* dev = (ImplDevice*)d;

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
  T_ASSERT_X( errCk(), "OpenGL error" );

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
    T_ASSERT_X( errCk(), "OpenGL error" );
    glTexImage2D( GL_TEXTURE_2D, 0, tex->format, p.width(), p.height(), 0,tex->format,
                  GL_UNSIGNED_BYTE, p.const_data() );
    T_ASSERT_X( errCk(), "OpenGL error" );
    if( mips )
      glGenerateMipmap( GL_TEXTURE_2D );
    T_ASSERT_X( errCk(), "OpenGL error" );
    } else {
    loadTextureS3TC(d, (AbstractAPI::Texture*)tex, p, mips, compress);
    }

  if( glGetError()==GL_INVALID_VALUE ){
    glDeleteTextures( 1, &tex->id );
    Log::e("OpenGL: unable to alloc texture");
    dev->texPool.free(tex);
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
#ifndef __MOBILE_PLATFORM__
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

#ifdef __MOBILE_PLATFORM__
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
    GL_STATIC_DRAW,
    GL_DYNAMIC_DRAW
    };

  Detail::GLBuffer *vbo = dev->bufPool.alloc();
  vbo->offset = 0;
  vbo->size   = 0;
  vbo->vertexCount = size;
  vbo->byteCount   = size*elSize;

  if( dev->useVBA(u) ){
    vbo->vba.insert( vbo->vba.begin(),
                     (const char*)src,
                     (const char*)src+size*elSize );
    vbo->id = 0;
    } else {
    if( dev->dynVbo.freed.size() ){
      vbo->id = *dev->dynVbo.freed.begin();
      dev->dynVbo.freed.erase( dev->dynVbo.freed.begin() );
      } else {
      glGenBuffers( 1, &vbo->id );
      }

    glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
    glBufferData( GL_ARRAY_BUFFER,
                  size*elSize, src,
                  gu[u] );

    if( u==BU_Dynamic ){
      dev->dynVbo.used.insert(vbo->id);
      }

    T_ASSERT_X( errCk(), "OpenGL error" );
    }

  return (AbstractAPI::VertexBuffer*)vbo;
  }

void Opengl2x::deleteVertexBuffer(  AbstractAPI::Device *d,
                                    AbstractAPI::VertexBuffer*v ) const{
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  if( vbo->id==0 ){
    //vbo->vba.clear();
    } else {
    if( dev->dynVbo.used.find(vbo->id)!=dev->dynVbo.used.end() ){
      glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      glBufferData( GL_ARRAY_BUFFER, 1, 0, GL_DYNAMIC_DRAW );

      dev->dynVbo.used .erase ( vbo->id );
      dev->dynVbo.freed.insert( vbo->id );
      } else {
      if( glIsBuffer(vbo->id) ){
        if( dev->curVBO==vbo->id ){
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          dev->curVBO = 0;
          }

        glDeleteBuffers( 1, &vbo->id );
        } else {
        T_ASSERT_X(0, "bad glDeleteBuffers call");
        }
      }
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
  ibo->vertexCount = size;
  ibo->byteCount   = size*elSize;

  if( false && dev->useVBA(u) ){
    ibo->vba.insert( ibo->vba.begin(),
                     (const char*)src, (const char*)src+size*elSize );
    ibo->id = 0;
    } else {
    glGenBuffers( 1, &ibo->id );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo->id );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                  size*elSize, src,
                  gu[u] );
    }

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
  T_ASSERT( offset+size <= vbo->byteCount );

  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  //vbo->mappedData = (char*)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );

  if( vbo->id==0 ){
    return &vbo->vba[ offset ];
    }

  if( !dev->lbUseed ){
    dev->lbUseed = true;
    dev->tmpLockBuffer.reserve(4*1024);
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

  if( vbo->id==0 )
    return;

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

void Opengl2x::updateBuffer(AbstractAPI::Device* d,
                             AbstractAPI::VertexBuffer* v,
                             const void* data, unsigned offset, unsigned size) const {
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  if( vbo->id==0 )
    return;

  glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void* Opengl2x::lockBuffer( AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer * v,
                            unsigned offset, unsigned size) const {
  (void)size;

  if( !setDevice(d) ) return 0;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  T_ASSERT( offset+size <= vbo->byteCount );

  if( vbo->id==0 ){
    return &vbo->vba[ offset ];
    }

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

  if( vbo->id==0 )
    return;

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

void Opengl2x::updateBuffer( AbstractAPI::Device* d,
                             AbstractAPI::IndexBuffer* v,
                             const void* data, unsigned offset, unsigned size) const {
  if( !setDevice(d) ) return;

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  if( vbo->id==0 )
    return;

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offset, size, data );
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

AbstractShadingLang *Opengl2x::createShadingLang( AbstractAPI::Device *d ) const {
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
  ImplDevice* dev = (ImplDevice*)d;
  VertexDeclaration::Declarator *dx = dev->declPool.alloc(de);
  return (AbstractAPI::VertexDecl*)dx;
  }

void Opengl2x::deleteVertexDecl( AbstractAPI::Device *d,
                                 AbstractAPI::VertexDecl* de ) const {
  ImplDevice* dev = (ImplDevice*)d;

  VertexDeclaration::Declarator *dx = (VertexDeclaration::Declarator*)(de);
  if( dev->decl==dx )
    dev->decl = 0;

  dev->declPool.free(dx);
  }

void Opengl2x::setVertexDeclaration( AbstractAPI::Device *d,
                                     AbstractAPI::VertexDecl* de,
                                     size_t vsize ) const {
  if( !setDevice(d) ) return;

  if( dev->decl==(VertexDeclaration::Declarator*)de )
    return;

  dev->decl       = (VertexDeclaration::Declarator*)de;
  dev->vertexSize = GLsizei(vsize);

  if( dev->isPainting )
    setupAttrPtr( *dev->decl, 0, true, true, true );
  }

void Opengl2x::bindVertexBuffer(AbstractAPI::Device *d,
                                 AbstractAPI::VertexBuffer* b,
                                 size_t vsize ) const {
  if( !setDevice(d) ) return;

  dev->vbo        = *(GLuint*)b;
  dev->cVBO       = (Detail::GLBuffer*)b;
  dev->vertexSize = GLsizei(vsize);
  }

void Opengl2x::bindIndexBuffer( AbstractAPI::Device * d,
                                AbstractAPI::IndexBuffer * b ) const {
  if( !setDevice(d) ) return;

  dev->ibo  = *(GLuint*)b;
  dev->cIBO = (Detail::GLBuffer*)b;
  }

void Opengl2x::setupBuffers( char* vboOffsetIndex,
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
    } else {
    dev->curVBO            = dev->vbo;
    dev->curVboOffsetIndex = vboOffsetIndex;
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

  if( rebind )
    setupAttrPtr( *dev->decl, vboOffsetIndex, enable, on, bind );

  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void Opengl2x::setupAttrPtr( const Tempest::VertexDeclaration::Declarator& vd,
                             char* vboOffsetIndex,
                             bool enable, bool on, bool bind ) const {
  if( !dev->decl )
    return;

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

  char* stride = vboOffsetIndex;
  for( int i=0; i<vd.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e = vd[i];

    int usrAttr = -1;
    int loc = i;
    if( vd[i].usage==Usage::TexCoord ){
      loc = vd.size()+vd[i].index;
      } else
    if( vd[i].attrName.size() ){
      loc = vd.size()+int( vd.texCoordCount() )+usrAttr;
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

    if( bind ){
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

  if( dev->cVBO->id==0 ){
    setupBuffers( &dev->cVBO->vba[0], false, false, true );
    } else {
    setupBuffers( 0, false, false, true );
    }

  T_ASSERT( size_t(firstVertex+pCount) <= dev->cVBO->vertexCount );

  if( dev->curIBO!=0 ){
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    dev->curIBO = 0;
    }

  static const GLenum type[] = {
    GL_PATCHES,
    GL_POINTS,         // = 1,
    GL_LINES,          // = 2,
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = vertexCount(t, pCount);
  glDrawArrays( type[ t ], firstVertex, vpCount );
  //setupBuffers(0, false, false);
  T_ASSERT_X( errCk(), "OpenGL error" );
  }

void Opengl2x::drawIndexed( AbstractAPI::Device *de,
                            AbstractAPI::PrimitiveType t,
                            int vboOffsetIndex,
                            int iboOffsetIndex,
                            int pCount ) const {
  setDevice(de);

  if( !dev->ibo )
    return;

  bool rebind = !(dev->curIBO == dev->ibo && dev->curIboOffsetIndex==iboOffsetIndex );

  if( dev->cVBO->id==0 ){
    char *offset = &dev->cVBO->vba[0]+(size_t(vboOffsetIndex)*dev->vertexSize);
    setupBuffers( offset, false, false, true );
    } else {
    char *offset = (char*)(size_t(vboOffsetIndex)*dev->vertexSize);
    setupBuffers( offset, false, false, true );
    }

  T_ASSERT( size_t(iboOffsetIndex+pCount) <= dev->cIBO->vertexCount );

  if( rebind ){
    dev->curIBO            = dev->ibo;
    dev->curIboOffsetIndex = iboOffsetIndex;
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, dev->ibo );
    }
  T_ASSERT_X( errCk(), "OpenGL error" );

  static const GLenum type[] = {
    GL_PATCHES,
    GL_POINTS,        // = 1,
    GL_LINES,         // = 2,
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = vertexCount(t, pCount);
  glDrawElements( type[ t ],
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
  ImplDevice *dev = (ImplDevice*)d;
  return Size(dev->scrW, dev->scrH);
  }

void Opengl2x::setRenderState( AbstractAPI::Device *d,
                               const RenderState & r) const {
  if( !setDevice(d) ) return;

  GLenum cull[] = {
    GL_FRONT_AND_BACK,
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

  if( dev->renderState.cullFaceMode()!=r.cullFaceMode() ){
    if( r.cullFaceMode()==RenderState::CullMode::noCull){
      glDisable( GL_CULL_FACE );
      } else {
      glEnable ( GL_CULL_FACE );
      glCullFace( cull[ r.cullFaceMode() ] );
      }
    }

  /*
  if( !r.isZTest() && !r.isZWriting() )
    glDisable(GL_DEPTH_TEST); else
    glEnable(GL_DEPTH_TEST);
  */

#ifdef __MOBILE_PLATFORM__
  //a-test not supported
  //glEnable( GL_BLEND );
#else
  if( r.alphaTestMode()!=Tempest::RenderState::AlphaTestMode::Always ){
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( cmp[ r.alphaTestMode() ], GLclampf(r.alphaTestRef()) );
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

#ifndef __MOBILE_PLATFORM__
  GLenum rmode[] = {
    GL_FILL,
    GL_LINE,
    GL_POINT
    };

  glPolygonMode( GL_FRONT, rmode[r.frontPolygonRenderMode()] );
  glPolygonMode( GL_BACK,  rmode[r.backPolygonRenderMode()] );
#endif

  if( ((ImplDevice*)dev)->isWriteOnly(r) && 0 ){
    glColorMask(1,1,1,1);
    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glDepthMask(0);
    ((ImplDevice*)dev)->enableWriteOnlyRender( 1 );
    } else {
    ((ImplDevice*)dev)->enableWriteOnlyRender( 0 );

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
        glBlendFunc( GL_ONE, GL_ZERO );
        }
      }

    if( dev->renderState.isZTest() != r.isZTest() ||
        dev->renderState.getZTestMode()!=r.getZTestMode() ){
      if( r.isZTest() ){
        glEnable( GL_DEPTH_TEST );
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
