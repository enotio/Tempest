#include "opengl2x.h"

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
// #include <SDL.h>
#include <android/log.h>
#else
#include <windows.h>

#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <Tempest/CgOGL>
#include <Tempest/GLSL>
#include <Tempest/RenderState>

#include <map>
#include <set>
#include <iostream>
#include <Tempest/Pixmap>

#include <cassert>
#include <stdexcept>

#include "gltypes.h"

using namespace Tempest;

struct Opengl2x::Device{
#ifdef __ANDROID__

#else
  HDC   hDC;
  HGLRC hRC;
#endif

  int scrW, scrH;

  VertexDeclaration::Declarator * decl;
  GLuint vbo, curVBO;
  GLuint ibo;

  int vertexSize, curVboOffsetIndex;

  GLuint fboId, rboId;
  bool    isPainting;

  Detail::RenderTg target;
  };

void Opengl2x::errCk() const {
  GLenum err = glGetError();
  while( err!=GL_NO_ERROR ){
#ifndef __ANDROID__
    std::cout << "[OpenGL]: " << glewGetErrorString(err) << std::endl;
#else
    void* ierr = (void*)err;
    __android_log_print(ANDROID_LOG_DEBUG, "OpenGL", "error %p", ierr);
#endif
    err = glGetError();
    }
  }

Opengl2x::Opengl2x( ShaderLang sl ):impl(0){
  shaderLang = sl;
  // impl   = (Opengl2xImpl*)Direct3DCreate9( D3D_SDK_VERSION );
  // data   = new Data();
  }

Opengl2x::~Opengl2x(){
  // LPDIRECT3D9 dev = LPDIRECT3D9(impl);
  //delete data;
  // dev->Release();
  }

AbstractAPI::Device* Opengl2x::createDevice(void *hwnd, const Options &opt) const {
  Device * dev = new Device();
  dev->isPainting = false;
  memset( &dev->target, 0, sizeof(dev->target) );

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

  if( !GLEW_VERSION_2_1 )
    if( glewInit()!=GLEW_OK || !GLEW_VERSION_2_1) {
      return 0;
      }
#endif

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenFramebuffers(1, &dev->fboId);

  reset( (AbstractAPI::Device*)dev, hwnd, opt );
  setRenderState( (AbstractAPI::Device*)dev, Tempest::RenderState() );
  return (AbstractAPI::Device*)dev;
  }

void Opengl2x::deleteDevice(AbstractAPI::Device *d) const {
  setDevice(d);
#ifndef __ANDROID__
  Device* dev = (Device*)d;

  glDeleteFramebuffers (1, &dev->fboId);

  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( dev->hRC );
#endif
  }

void Opengl2x::setDevice( AbstractAPI::Device * d ) const {
  dev = (Device*)d;
  errCk();
  }

void Opengl2x::clear(AbstractAPI::Device *d,
                      const Color& cl, float z, unsigned stencil ) const {
  setDevice(d);

  glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
#ifdef __ANDROID__
  glClearDepthf( z );
#else
  glClearDepth( z );
#endif
  glClearStencil( stencil );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  }

void Opengl2x::clear(AbstractAPI::Device *d, const Color &cl) const  {
  setDevice(d);

  glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
  glClear( GL_COLOR_BUFFER_BIT );
  }

void Opengl2x::clearZ(AbstractAPI::Device *d, float z ) const  {
  setDevice(d);

#ifdef __ANDROID__
  glClearDepthf( z );
#else
  glClearDepth( z );
#endif
  glClear( GL_DEPTH_BUFFER_BIT );
  }

void Opengl2x::clearStencil( AbstractAPI::Device *d, unsigned s ) const  {
  setDevice(d);

  glClearStencil( s );
  glClear( GL_STENCIL_BUFFER_BIT );
  }

void Opengl2x::clear(AbstractAPI::Device *d,
                      float z, unsigned s ) const {
  setDevice(d);

#ifdef __ANDROID__
  glClearDepthf( z );
#else
  glClearDepth( z );
#endif
  glClearStencil( s );

  glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  }

void Opengl2x::clear(AbstractAPI::Device *d,  const Color& cl, float z ) const{
  setDevice(d);

  glClearColor( cl.r(), cl.g(), cl.b(), cl.a() );
#ifdef __ANDROID__
  glClearDepthf( z );
#else
  glClearDepth( z );
#endif
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }

void Opengl2x::beginPaint( AbstractAPI::Device * d ) const {
  setDevice(d);

  assert( !dev->isPainting );
  dev->isPainting = true;
  setupBuffers( 0, true, true, false );
  setupFBO();
  }

void Opengl2x::endPaint  ( AbstractAPI::Device * d ) const{
  setDevice(d);

  dev->isPainting = false;
  setupBuffers( 0, false, true, false );
  dev->vbo    = 0;
  dev->ibo    = 0;
  dev->curVBO = 0;

  dev->curVboOffsetIndex = 0;
  }

void Opengl2x::setupFBO() const {
  const int maxMRT = Detail::RenderTg::maxMRT;
    
  GLint w = 0, h = 0;
  for( int i=0; i<maxMRT; ++i ){
    if( dev->target.color[i] ){
      Detail::GLTexture *tex = dev->target.color[i];
      w = tex->w;
      h = tex->h;
      }
    }

  if( w==0 || h==0 ){
    return;
    }

  if( !dev->target.depth ){
    glGenRenderbuffers( 1, &dev->rboId );
    glBindRenderbuffer( GL_RENDERBUFFER, dev->rboId);
    glRenderbufferStorage( GL_RENDERBUFFER,
                         #ifndef __ANDROID__
                           GL_DEPTH_STENCIL,
                         #else
                           GL_DEPTH_COMPONENT16,
                         #endif
                           w, h );
    glBindRenderbuffer( GL_RENDERBUFFER, 0);
    }

  //assert( glCheckFramebufferStatus( GL_FRAMEBUFFER )==GL_FRAMEBUFFER_COMPLETE );
  errCk();

  glBindFramebuffer(GL_FRAMEBUFFER, dev->fboId);
  for( int i=0; i<maxMRT; ++i ){
    if( dev->target.color[i] ){
      glBindTexture( GL_TEXTURE_2D, dev->target.color[i]->id );
      dev->target.color[i]->min = dev->target.color[i]->mag = GL_NEAREST;
      glTexParameteri( GL_TEXTURE_2D,
                       GL_TEXTURE_MIN_FILTER,
                       GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D,
                       GL_TEXTURE_MAG_FILTER,
                       GL_NEAREST );
      glBindTexture( GL_TEXTURE_2D, 0 );

      glFramebufferTexture2D( GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0+i,
                              GL_TEXTURE_2D,
                              dev->target.color[i]->id,
                              dev->target.mip[i] );
      dev->target.color[i]->mips = false;
      }
    }

  if( dev->target.depth ){
    glBindTexture( GL_TEXTURE_2D, dev->target.depth->id );
    dev->target.depth->min = dev->target.depth->mag = GL_NEAREST;
    dev->target.depth->mips = false;
    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_MIN_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glFramebufferTexture2D( GL_FRAMEBUFFER,
                            GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D,
                            dev->target.depth->id,
                            dev->target.depthMip );
    } else {
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,
                               0*dev->rboId );
    }

  int status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

  if( !(status==0 || status==GL_FRAMEBUFFER_COMPLETE) ){
    std::cout << std::hex << status << std::dec << std::endl;
    }

  assert( status==0 || status==GL_FRAMEBUFFER_COMPLETE );
  errCk();

  glViewport( 0, 0, w, h );
  errCk();
  }

void Opengl2x::setRenderTaget( AbstractAPI::Device  *d,
                               AbstractAPI::Texture *t,
                               int mip,
                               int mrtSlot ) const {
  setDevice(d);
  Detail::GLTexture *tex = (Detail::GLTexture*)t;
  dev->target.color[mrtSlot] = tex;
  dev->target.mip  [mrtSlot] = mip;
  }

void Opengl2x::unsetRenderTagets( AbstractAPI::Device *d,
                                  int /*count*/  ) const {
  setDevice(d);
  memset( &dev->target, 0, sizeof(dev->target) );

  glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                             GL_DEPTH_ATTACHMENT,
                             GL_RENDERBUFFER,
                             0 );
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if( dev->rboId )
    glDeleteRenderbuffers(1, &dev->rboId);
  //glDeleteFramebuffers (1, &dev->fboId);

  glViewport( 0, 0, dev->scrW, dev->scrH );
  errCk();
  }

AbstractAPI::StdDSSurface *Opengl2x::getDSSurfaceTaget( AbstractAPI::Device * ) const {
  return 0;
  }

void Opengl2x::retDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface * ) const {
  setDevice(d);
  dev->target.depth = 0;
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface *tx ) const {
  setDevice(d);
#ifndef __ANDROID__
  dev->target.depth = (Detail::GLTexture*)tx;
#endif
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::Texture *tx ) const {
  setDevice(d);
#ifndef __ANDROID__
  dev->target.depth = (Detail::GLTexture*)tx;
#endif
  }

bool Opengl2x::startRender( AbstractAPI::Device *,bool   ) const {
  return 1;
  }

bool Opengl2x::present( AbstractAPI::Device *d ) const {
#ifndef __ANDROID__
  Device* dev = (Device*)d;
  SwapBuffers( dev->hDC );
#else
  EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLSurface s    = eglGetCurrentSurface(EGL_DRAW);

  eglSwapBuffers(disp, s);
#endif
  return 0;
  }

bool Opengl2x::reset( AbstractAPI::Device *d,
                      void* hwnd,
                      const Options &opt ) const {
  setDevice(d);

#ifndef __ANDROID__
  RECT rectWindow;
  GetClientRect( HWND(hwnd), &rectWindow);

  int w = rectWindow.right  - rectWindow.left;
  int h = rectWindow.bottom - rectWindow.top;

  dev->scrW = w;
  dev->scrH = h;
  glViewport(0,0, w,h);
#else
  EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLSurface s    = eglGetCurrentSurface(EGL_DRAW);

  EGLint w;
  EGLint h;
  eglQuerySurface( disp, s, EGL_WIDTH,  &w );
  eglQuerySurface( disp, s, EGL_HEIGHT, &h );
  
  dev->scrW = w;
  dev->scrH = h;
  glViewport(0,0,w,h);
#endif
  return 1;
  }

AbstractAPI::Texture *Opengl2x::createTexture( AbstractAPI::Device *d,
                                               const Pixmap &p,
                                               bool mips ) const {
  if( p.width()==0 || p.height()==0 )
    return 0;

  setDevice(d);

  Detail::GLTexture* tex = new Detail::GLTexture;
  glGenTextures(1, &tex->id);
  glBindTexture(GL_TEXTURE_2D, tex->id);

  tex->min = tex->mag = GL_NEAREST;
  tex->mips = mips;

  tex->w = p.width();
  tex->h = p.height();

  if( p.hasAlpha() )
    tex->format = GL_RGBA; else
    tex->format = GL_RGB;

  glTexImage2D( GL_TEXTURE_2D, 0, tex->format, p.width(), p.height(), 0,tex->format,
                GL_UNSIGNED_BYTE, p.const_data() );

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST );

  if( mips )
    glGenerateMipmap( GL_TEXTURE_2D );

  return ((AbstractAPI::Texture*)tex);
  }

AbstractAPI::Texture *Opengl2x::recreateTexture( AbstractAPI::Device *d,
                                                 AbstractAPI::Texture *oldT,
                                                 const Pixmap &p,
                                                 bool mips) const {
  if( oldT==0 )
    return createTexture(d, p, mips);

  setDevice(d);
  Detail::GLTexture* tex = 0;
  Detail::GLTexture* old = (Detail::GLTexture*)(oldT);

  GLenum format = GL_RGB;
  if( p.hasAlpha() )
    format = GL_RGBA;

  if( int(old->w)==p.width() &&
      int(old->h)==p.height() &&
      old->format==format &&
      old->mips  ==mips ){
    tex = old;
    } else {
    deleteTexture(d, oldT);
    return createTexture( d, p, mips );
    }

  glBindTexture(GL_TEXTURE_2D, tex->id);
  glTexImage2D( GL_TEXTURE_2D, 0, tex->format, p.width(), p.height(), 0, tex->format,
                GL_UNSIGNED_BYTE, p.const_data() );

  if( mips )
    glGenerateMipmap( GL_TEXTURE_2D );

  return (AbstractAPI::Texture*)tex;
  }

AbstractAPI::Texture* Opengl2x::createTexture( AbstractAPI::Device *d,
                                               int w, int h,
                                               int mips,
                                               AbstractTexture::Format::Type f,
                                               TextureUsage u  ) const {
  if( w==0 || h==0 )
    return 0;

  (void)u;
  setDevice(d);

  Detail::GLTexture* tex = new Detail::GLTexture;
  glGenTextures(1, &tex->id);
  glBindTexture(GL_TEXTURE_2D, tex->id);

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

    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    GL_LUMINANCE, //d
    GL_LUMINANCE, //d
    GL_LUMINANCE, //d
    GL_LUMINANCE, //ds

    GL_RGB,
    GL_RGBA
    };
  
  errCk();

  tex->min = tex->mag = GL_NEAREST;
  tex->mips = mips;

  tex->w = w;
  tex->h = h;
  tex->format = format[f];

#ifdef __ANDROID__
  glTexImage2D( GL_TEXTURE_2D, 0,
                format[f], w, h, 0,
                format[f],
                GL_UNSIGNED_BYTE, 0 );
#else
  glTexImage2D( GL_TEXTURE_2D, 0,
                format[f], w, h, 0,
                inputFormat[f],
                GL_UNSIGNED_BYTE, 0 );
#endif
  errCk();

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST );

  if( mips!=0 )
    glGenerateMipmap(GL_TEXTURE_2D);
  
  errCk();
  return (AbstractAPI::Texture*)tex;
  }

void Opengl2x::deleteTexture( AbstractAPI::Device *d,
                              AbstractAPI::Texture *t ) const {
  if( !t )
    return;

  setDevice(d);
  Detail::GLTexture* tex = (Detail::GLTexture*)t;
  glDeleteTextures( 1, &tex->id );
  delete tex;
  
  errCk();
  }

AbstractAPI::VertexBuffer*
     Opengl2x::createVertexbuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize ) const{
  setDevice(d);

  Detail::GLBuffer *vbo = new Detail::GLBuffer;
  glGenBuffers( 1, &vbo->id );
  glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  glBufferData( GL_ARRAY_BUFFER,
                size*elSize, 0,
                GL_STATIC_DRAW );
  errCk();

  return (AbstractAPI::VertexBuffer*)vbo;
  }

void Opengl2x::deleteVertexBuffer(  AbstractAPI::Device *d,
                                    AbstractAPI::VertexBuffer*v ) const{
  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;
  glDeleteBuffers( 1, &vbo->id );
  delete vbo;

  errCk();
  }

AbstractAPI::IndexBuffer*
     Opengl2x::createIndexbuffer( AbstractAPI::Device *d,
                                  size_t size, size_t elSize ) const {
  setDevice(d);

  Detail::GLBuffer *ibo = new Detail::GLBuffer;
  glGenBuffers( 1, &ibo->id );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo->id );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                size*elSize, 0,
                GL_STATIC_DRAW );
  errCk();

  return (AbstractAPI::IndexBuffer*)ibo;
  }

void Opengl2x::deleteIndexBuffer( AbstractAPI::Device *d,
                                  AbstractAPI::IndexBuffer* v) const {
  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;
  glDeleteBuffers( 1, &vbo->id );
  delete vbo;

  errCk();
  }

void* Opengl2x::lockBuffer( AbstractAPI::Device *d,
                            AbstractAPI::VertexBuffer * v,
                            unsigned offset,
                            unsigned size ) const {
  (void)size;

  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  //vbo->mappedData = (char*)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
  vbo->mappedData = new char[size];
  vbo->offset = offset;
  vbo->size   = size;

  errCk();
  return vbo->mappedData;
  }

void Opengl2x::unlockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::VertexBuffer* v) const {  
  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ARRAY_BUFFER, vbo->offset, vbo->size, vbo->mappedData );

  delete[] vbo->mappedData;
  vbo->mappedData = 0;

  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
  //glUnmapBuffer( GL_ARRAY_BUFFER );
  errCk();
  }

void* Opengl2x::lockBuffer( AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer * v,
                            unsigned offset, unsigned size) const {
  (void)size;

  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  vbo->mappedData = new char[size];
  vbo->offset = offset;
  vbo->size   = size;

  errCk();
  return vbo->mappedData;
  }

void Opengl2x::unlockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::IndexBuffer* v) const {
  setDevice(d);

  Detail::GLBuffer *vbo = (Detail::GLBuffer*)v;

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->id );
  glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, vbo->offset, vbo->size, vbo->mappedData );

  delete[] vbo->mappedData;
  vbo->mappedData = 0;

  //glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, *vbo );
  //glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
  errCk();
  }

const AbstractShadingLang*
        Opengl2x::createShadingLang( AbstractAPI::Device *d ) const {
  setDevice(d);
  AbstractAPI::OpenGL2xDevice *dev =
      reinterpret_cast<AbstractAPI::OpenGL2xDevice*>(d);

#ifndef __ANDROID__
  if( shaderLang==Cg )
    return new CgOGL( dev );
#endif

  if( shaderLang==GLSL )
    return new Tempest::GLSL( dev );

#ifndef __ANDROID__
  throw std::logic_error("invalid shaders lang");
#else
  return 0;
#endif
  }

void Opengl2x::deleteShadingLang( const AbstractShadingLang * l ) const {
  delete l;
  }

AbstractAPI::VertexDecl*
      Opengl2x::createVertexDecl( AbstractAPI::Device *,
                                  const VertexDeclaration::Declarator &de ) const {
  VertexDeclaration::Declarator *d = new VertexDeclaration::Declarator(de);
  return (AbstractAPI::VertexDecl*)d;
  }

void Opengl2x::deleteVertexDecl( AbstractAPI::Device *,
                                 AbstractAPI::VertexDecl* de ) const {
  VertexDeclaration::Declarator *d = (VertexDeclaration::Declarator*)(de);
  delete d;
  }

void Opengl2x::setVertexDeclaration( AbstractAPI::Device *d,
                                     AbstractAPI::VertexDecl* de ) const {
  setDevice(d);
  dev->decl = (VertexDeclaration::Declarator*)de;

  if( dev->isPainting )
    setupBuffers( 0, true, true, false );
  }

void Opengl2x::bindVertexBuffer( AbstractAPI::Device *d,
                                 AbstractAPI::VertexBuffer* b,
                                 int vsize ) const {
  setDevice(d);

  dev->vbo        = *(GLuint*)b;
  dev->vertexSize = vsize;
  }

void Opengl2x::bindIndexBuffer( AbstractAPI::Device * d,
                                AbstractAPI::IndexBuffer * b ) const {
  setDevice(d);

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

    glBindBuffer( GL_ARRAY_BUFFER, dev->vbo );
    }

  static const GLenum vfrm[] = {
    GL_FLOAT, //double0 = 0, // just trick
    GL_FLOAT, //double1 = 1,
    GL_FLOAT, //double2 = 2,
    GL_FLOAT, //double3 = 3,
    GL_FLOAT, //double4 = 4,

    GL_UNSIGNED_BYTE, //color  = 5,
    GL_SHORT,//short2 = 6,
    GL_SHORT,//short4 = 7
    };

  static const int counts[] = {
    0, 1, 2, 3, 4, 4, 2, 4
    };

  static const int strides[] = {
    0, 4*1, 4*2, 4*3, 4*4, 4, 2*2, 2*4
    };

  size_t stride = vboOffsetIndex;

  const Tempest::VertexDeclaration::Declarator& vd
      = *(const Tempest::VertexDeclaration::Declarator*)dev->decl;
  for( int i=0; i<vd.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e = vd[i];

    int loc = i;
    if( vd[i].usage==Usage::TexCoord ){
      loc = vd.size()+vd[i].index;
      }

    int count  = counts[e.component];
    GLenum frm =   vfrm[e.component];

    if( enable ){
      if( on ){
        glEnableVertexAttribArray(loc);
        } else {
        glDisableVertexAttribArray(loc);
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
/*
  for( int i=0; i<d.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e = d[i];
    int count  = counts[e.component];
    GLenum frm =   vfrm[e.component];

    if( on ){
      glEnableVertexAttribArray(i);
      if( e.component==Decl::color )
        glVertexAttribPointer( i, count, frm, GL_TRUE,
                               dev->vertexSize, (void*)stride );
      else
        glVertexAttribPointer( i, count, frm, GL_FALSE,
                               dev->vertexSize, (void*)stride );

      } else {
      glDisableVertexAttribArray(i);
      }
    stride += strides[e.component];
    }*/

  errCk();
  }

void Opengl2x::draw( AbstractAPI::Device *de,
                     AbstractAPI::PrimitiveType t,
                     int firstVertex, int pCount ) const {
  setDevice(de);
  setupBuffers( 0, false, false, true );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  static const GLenum type[] = {
  #ifdef __ANDROID__
    GL_POINTS,         // = 1,
    GL_LINES,          // = 2,
  #else
    GL_POINT,         // = 1,
    GL_LINE,          // = 2,
  #endif
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = pCount*3;
  if( t==AbstractAPI::TriangleStrip ||
      t==AbstractAPI::TriangleFan ||
      t==AbstractAPI::LinesStrip  )
    vpCount = pCount+2;

  glDrawArrays( type[ t-1 ], firstVertex, vpCount );
  //setupBuffers(0, false, false);
  errCk();
  }

//unstable
void Opengl2x::drawIndexed(  AbstractAPI::Device *de,
                             AbstractAPI::PrimitiveType t,
                             int vboOffsetIndex,
                             int minIndex,
                             int vertexCount,
                             int firstIndex,
                             int pCount ) const {
  setDevice(de);
  if( !dev->ibo )
    return;

  setupBuffers( vboOffsetIndex*dev->vertexSize, false, false, true );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, dev->ibo );

  static const GLenum type[] = {
  #ifdef __ANDROID__
    GL_POINTS,         // = 1,
    GL_LINES,          // = 2,
  #else
    GL_POINT,         // = 1,
    GL_LINE,          // = 2,
  #endif
    GL_LINE_STRIP,    // = 3,
    GL_TRIANGLES,     // = 4,
    GL_TRIANGLE_STRIP,// = 5,
    GL_TRIANGLE_FAN,  // = 6
    };

  int vpCount = pCount*3;
  if( t==AbstractAPI::TriangleStrip ||
      t==AbstractAPI::TriangleFan ||
      t==AbstractAPI::LinesStrip  )
    vpCount = pCount+2;

  if( firstIndex==0 ){
    glDrawElements( type[ t-1 ], vpCount, GL_UNSIGNED_SHORT,
                    (void*)(firstIndex*sizeof(short)) );
    } else {
#ifndef __ANDROID__
    glDrawRangeElements( type[ t-1 ],
                         firstIndex,
                         firstIndex+vertexCount,
                         vpCount,
                         GL_UNSIGNED_SHORT,
                         0 );
#endif
    }

  //setupBuffers( firstIndex, false, false );
  errCk();
  }

void Opengl2x::setRenderState( AbstractAPI::Device *d,
                               const RenderState & r) const {
  setDevice(d);

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

  glCullFace( cull[ r.cullFaceMode() ] );
  glDepthFunc( cmp[ r.getZTestMode() ] );

  if( r.isZTest() )
    glEnable ( GL_DEPTH_TEST ); else
    glDisable( GL_DEPTH_TEST );

  glDepthMask( r.isZWriting() );
  
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

  if( r.isBlend() ){
    glEnable( GL_BLEND );
    glBlendFunc( blend[ r.getBlendSFactor() ], blend[ r.getBlendDFactor() ] );
    } else {
    glDisable( GL_BLEND );
    }

  bool w[4];
  r.getColorMask( w[0], w[1], w[2], w[3] );
  glColorMask( w[0], w[1], w[2], w[3] );

  errCk();
  }
