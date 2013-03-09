#include "opengl2x.h"

#ifdef __ANDROID__
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <SDL.h>
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

using namespace Tempest;

struct Opengl2x::Device{
#ifdef __ANDROID__

#else
  HDC   hDC;
  HGLRC hRC;
#endif

  int scrW, scrH;

  VertexDeclaration::Declarator * decl;
  GLuint vbo;
  GLuint ibo;

  int vertexSize;

  GLuint fboId, rboId;
  };

void Opengl2x::errCk() const {
#ifndef __ANDROID__
  GLenum err = glGetError();
  while( err!=GL_NO_ERROR ){
    std::cout << "[OpenGL]: " << glewGetErrorString(err) << std::endl;
    err = glGetError();
    }
#endif
  }

struct Opengl2x::Texture{
  GLuint id;
  int w,h;
  };

struct Opengl2x::Buffer{
  GLuint id;
  char * mappedData;

  unsigned offset, size;
  };

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
#ifdef __ANDROID__

#else
  GLuint PixelFormat;

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

  pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion   = 1;
  pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;

  Device * dev = new Device();

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

  reset( (AbstractAPI::Device*)dev, hwnd, opt );
  setRenderState( (AbstractAPI::Device*)dev, Tempest::RenderState() );
  return (AbstractAPI::Device*)dev;
  }

void Opengl2x::makePresentParams( void * p, void *hWnd,
                                  const Options &opt ) const{
  /*
  LPDIRECT3D9 D3D = LPDIRECT3D9(impl);

  D3DDISPLAYMODE d3ddm;
  D3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

  D3DPRESENT_PARAMETERS& d3dpp = *(reinterpret_cast<D3DPRESENT_PARAMETERS*>(p));
  ZeroMemory( &d3dpp, sizeof(d3dpp) );

  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
//GetSystemMetrics
  d3dpp.BackBufferWidth  = rectWindow.right  - rectWindow.left;
  d3dpp.BackBufferHeight = rectWindow.bottom - rectWindow.top;

  d3dpp.Windowed               = opt.windowed;
  d3dpp.BackBufferFormat       = d3ddm.Format;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

  d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;

  if( opt.vSync )
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE; else
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;


  if( !d3dpp.Windowed ){
    d3dpp.BackBufferWidth  = d3ddm.Width;
    d3dpp.BackBufferHeight = d3ddm.Height;

    d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
    }
    */
  }

void Opengl2x::deleteDevice(AbstractAPI::Device *d) const {
#ifndef __ANDROID__
  Device* dev = (Device*)d;

  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( dev->hRC );
#endif
  }

void Opengl2x::setDevice( AbstractAPI::Device * d ) const {
  dev = (Device*)d;
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

void Opengl2x::beginPaint( AbstractAPI::Device * ) const {
  }

void Opengl2x::endPaint  ( AbstractAPI::Device * ) const{
  dev->vbo = 0;
  dev->ibo = 0;
  }

void Opengl2x::setRenderTaget( AbstractAPI::Device  *d,
                               AbstractAPI::Texture *t,
                               int mip,
                               int mrtSlot ) const {
  setDevice(d);
  Texture *tex = (Texture*)t;

  GLint w = tex->w,
        h = tex->h;

  glBindTexture(GL_TEXTURE_2D, *(GLuint*)t );
  glBindTexture(GL_TEXTURE_2D, 0 );
  errCk();


  glGenFramebuffers (1, &dev->fboId);
  glGenRenderbuffers(1, &dev->rboId);
  errCk();

  glBindRenderbuffer( GL_RENDERBUFFER, dev->rboId);
  glRenderbufferStorage( GL_RENDERBUFFER,
                       #ifndef __ANDROID__
                         GL_DEPTH_STENCIL,
                       #else
                         GL_DEPTH_COMPONENT16,
                       #endif
                         w, h );

  glBindRenderbuffer( GL_RENDERBUFFER, 0);
  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  assert( status==GL_FRAMEBUFFER_COMPLETE );

  errCk();

  glBindFramebuffer(GL_FRAMEBUFFER, dev->fboId);
  glFramebufferTexture2D( GL_FRAMEBUFFER,
                          GL_COLOR_ATTACHMENT0+mrtSlot,
                          GL_TEXTURE_2D,
                          *(GLuint*)t,
                          mip );

  glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                           #ifndef __ANDROID__
                             GL_DEPTH_STENCIL_ATTACHMENT,
                           #else
                             GL_DEPTH_ATTACHMENT,
                           #endif
                             GL_RENDERBUFFER,
                             dev->rboId );
  errCk();

  glViewport( 0, 0, w, h );
  errCk();
  }

void Opengl2x::unsetRenderTagets( AbstractAPI::Device *d,
                                  int /*count*/  ) const {
  setDevice(d);
  glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                           #ifndef __ANDROID__
                             GL_DEPTH_STENCIL_ATTACHMENT,
                           #else
                             GL_DEPTH_ATTACHMENT,
                           #endif
                             GL_RENDERBUFFER,
                             0 );
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glDeleteRenderbuffers(1, &dev->rboId);
  glDeleteFramebuffers (1, &dev->fboId);

  glViewport( 0, 0, dev->scrW, dev->scrH );
  errCk();
  }

AbstractAPI::StdDSSurface *Opengl2x::getDSSurfaceTaget( AbstractAPI::Device * ) const {
  return 0;
  }

void Opengl2x::retDSSurfaceTaget( AbstractAPI::Device *,
                                  AbstractAPI::StdDSSurface * ) const {
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface *tx ) const {
  }

void Opengl2x::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::Texture *tx ) const {
  }

bool Opengl2x::startRender( AbstractAPI::Device *d,
                            bool isLost  ) const {
  return 1;
  }

bool Opengl2x::present( AbstractAPI::Device *d ) const {
#ifndef __ANDROID__
  Device* dev = (Device*)d;
  SwapBuffers( dev->hDC );
#endif
  return 0;
  }

bool Opengl2x::reset( AbstractAPI::Device *d,
                      void* hwnd,
                      const Options &opt ) const {
#ifndef __ANDROID__
  setDevice(d);

  RECT rectWindow;
  GetClientRect( HWND(hwnd), &rectWindow);

  int w = rectWindow.right  - rectWindow.left;
  int h = rectWindow.bottom - rectWindow.top;

  dev->scrW = w;
  dev->scrH = h;
  glViewport(0,0, w,h);
#endif
  return 1;
  }

AbstractAPI::Texture *Opengl2x::createTexture( AbstractAPI::Device *d,
                                               const Pixmap &p,
                                               bool mips ) const {
  if( p.width()==0 || p.height()==0 )
    return 0;

  setDevice(d);

  GLuint* tex = new GLuint;
  glGenTextures(1, tex);
  glBindTexture(GL_TEXTURE_2D, *tex);

  if( p.hasAlpha() )
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, p.width(), p.height(), 0,GL_RGBA,
                  GL_UNSIGNED_BYTE, p.const_data() ); else
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, p.width(), p.height(), 0,GL_RGB,
                  GL_UNSIGNED_BYTE, p.const_data() );

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR );

  if( mips )
    glGenerateMipmap( GL_TEXTURE_2D );

  return ((AbstractAPI::Texture*)tex);
  }

AbstractAPI::Texture *Opengl2x::recreateTexture( AbstractAPI::Device *d,
                                                 AbstractAPI::Texture *oldT,
                                                 const Pixmap &p,
                                                 bool mips) const {
  assert(0);

  if( oldT==0 )
    return createTexture(d, p, mips);
/*
  D3DLOCKED_RECT lockedRect;
  LPDIRECT3DDEVICE9 dev = LPDIRECT3DDEVICE9(d);
  LPDIRECT3DTEXTURE9 tex = 0;
  LPDIRECT3DTEXTURE9 old = LPDIRECT3DTEXTURE9(oldT);

  D3DFORMAT format = D3DFMT_X8R8G8B8;
  if( p.hasAlpha() )
    format = D3DFMT_A8R8G8B8;

  D3DSURFACE_DESC desc;
  if( FAILED( old->GetLevelDesc(0, &desc) ) ){
    deleteTexture(d, oldT);
    return 0;
    }

  if( int(desc.Width)==p.width() &&
      int(desc.Height)==p.height() &&
      desc.Format==format ){
    tex = old;
    } else {
    deleteTexture(d, oldT);

    if (FAILED(dev->CreateTexture( p.width(), p.height(), 0,
                                   D3DUSAGE_AUTOGENMIPMAP, format,
                                   D3DPOOL_MANAGED,
                                   &tex, NULL))) {
      return 0;
      }
    }

  if (FAILED( tex->LockRect(0, &lockedRect, 0, 0))) {
    return 0;
    }

  unsigned char *dest = (unsigned char*) lockedRect.pBits;

  for( int i=0; i<p.width(); ++i )
    for( int r=0; r<p.height(); ++r ){
      unsigned char * t = &dest[ 4*(i + r*p.width()) ];
      const Pixmap::Pixel s = p.at(i,r);
      t[2] = s.r;
      t[1] = s.g;
      t[0] = s.b;
      t[3] = s.a;
      }

  tex->UnlockRect(0);

  if( mips )
    tex->GenerateMipSubLevels();

  //data->tex.insert( tex );
  return ((AbstractAPI::Texture*)tex);
  */
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

  Texture* tex = new Texture;
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

    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,
    GL_DEPTH_COMPONENT32,
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
    GL_LUMINANCE_ALPHA,

    GL_RGB,
    GL_RGBA
    };
#endif

  tex->w = w;
  tex->h = h;

  glTexImage2D( GL_TEXTURE_2D, 0,
                format[f], w, h, 0,
                GL_RGBA,
                GL_UNSIGNED_BYTE, 0 );

  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR );

  if( mips!=0 )
    glGenerateMipmap(GL_TEXTURE_2D);

  return (AbstractAPI::Texture*)tex;
  }

void Opengl2x::deleteTexture( AbstractAPI::Device *d,
                              AbstractAPI::Texture *t ) const {
  setDevice(d);
  Texture* tex = (Texture*)t;
  glDeleteTextures( 1, &tex->id );
  }

AbstractAPI::VertexBuffer*
     Opengl2x::createVertexbuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize ) const{
  setDevice(d);

  Buffer *vbo = new Buffer;
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

  Buffer *vbo = (Buffer*)v;
  glDeleteBuffers( 1, &vbo->id );
  delete vbo;

  errCk();
  }

AbstractAPI::IndexBuffer*
     Opengl2x::createIndexbuffer( AbstractAPI::Device *d,
                                  size_t size, size_t elSize ) const {
  setDevice(d);

  Buffer *ibo = new Buffer;
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

  Buffer *vbo = (Buffer*)v;
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

  Buffer *vbo = (Buffer*)v;

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

  Buffer *vbo = (Buffer*)v;

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

  Buffer *vbo = (Buffer*)v;

  vbo->mappedData = new char[size];
  vbo->offset = offset;
  vbo->size   = size;

  errCk();
  return vbo->mappedData;
  }

void Opengl2x::unlockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::IndexBuffer* v) const {
  setDevice(d);

  Buffer *vbo = (Buffer*)v;

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
  AbstractAPI::OpenGL2xDevice *dev =
      reinterpret_cast<AbstractAPI::OpenGL2xDevice*>(d);

#ifndef __ANDROID__
  if( shaderLang==Cg )
    return new CgOGL( dev );
#endif

  if( shaderLang==GLSL )
    return new Tempest::GLSL( dev, false );

  if( shaderLang==GLSL_cgc_gen )
    return new Tempest::GLSL( dev, true );

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

void Opengl2x::setupBuffers( int vboOffsetIndex ) const {
  if( !dev->vbo )
    return;

  glBindBuffer( GL_ARRAY_BUFFER, dev->vbo );

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
  /*
  static const GLenum uType[] = {
    Position = 0,
    BlendWeight,   // 1
    BlendIndices,  // 2
    Normal,        // 3
    PSize,         // 4
    TexCoord,      // 5
    Tangent,       // 6
    BiNormal,      // 7
    TessFactor,    // 8
    PositionT,     // 9
    Color,         // 10
    Fog,           // 11
    Depth,         // 12
    Sample,        // 13
    Count
    };*/

  static const int counts[] = {
    0, 1, 2, 3, 4, 4, 2, 4
    };

  static const int strides[] = {
    0, 4*1, 4*2, 4*3, 4*4, 4, 2*2, 2*4
    };

  const VertexDeclaration::Declarator & d = *dev->decl;

  size_t stride = vboOffsetIndex;

  for( int i=0; i<d.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e = d[i];
    int count  = counts[e.component];
    GLenum frm =   vfrm[e.component];

    if( e.usage == Tempest::Usage::Position ||
        e.usage == Tempest::Usage::PositionT ){
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer( count, frm, dev->vertexSize, (void*)stride );
      }

    if( e.usage == Tempest::Usage::TexCoord  ){
      glClientActiveTexture( GL_TEXTURE0 + e.index );
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer( count, frm, dev->vertexSize, (void*)stride );
      }

    if( e.usage == Tempest::Usage::Color  ){
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer( count, frm, dev->vertexSize, (void*)stride );
      }

    stride += strides[e.component];
    }

  errCk();
  }

void Opengl2x::draw( AbstractAPI::Device *de,
                     AbstractAPI::PrimitiveType t,
                     int firstVertex, int pCount ) const {
  setDevice(de);
  setupBuffers(0);
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

  setupBuffers( firstIndex );
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
  if( r.alphaTestMode()!=Tempest::RenderState::AlphaTestMode::Always ){
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( cmp[ r.alphaTestMode() ], r.alphaTestRef() );
    } else {
    glDisable( GL_ALPHA_TEST );
    }

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
  }
