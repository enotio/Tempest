#include "opengl4x.h"

#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <windows.h>
#endif

#ifndef __ANDROID__
#include <GL/gl.h>
#include "ogl/glcorearb.h"
#ifdef __WINDOWS__
#include "glfn.h"
#endif

#include "gltypes.h"

#include <Tempest/Log>

using namespace Tempest;

#ifdef __WINDOWS__
  typedef HGLRC (GLAPIENTRY *PFNWGLCREATECONTEXTATTRIBSARBPROC)
      (HDC hDC, HGLRC hshareContext,const int *attribList);
#endif

static void* getAddr( const char* name ){
#ifdef __WINDOWS__
  return (void*)wglGetProcAddress(name);
#endif

#ifdef __ANDROID__
  return (void*)eglGetProcAddress( name );
#endif

#ifdef __LINUX__
  return (void*)glXGetProcAddress( (const GLubyte*)name );
#endif
  return 0;
  }

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

struct Opengl4x::ImplDevice: Detail::ImplDeviceBase {
  PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays;
  PFNGLBINDVERTEXARRAYPROC    glBindVertexArray;
  PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

  GLuint vao = 0;
  };

Opengl4x::Opengl4x() {

  }

AbstractAPI::Device *Opengl4x::createDevice( void *hwnd,
                                             const AbstractAPI::Options &opt ) const {
  ImplDevice* dev = new ImplDevice();

  if( !createContext(dev, hwnd, opt) )
    return 0;

#ifdef __LINUX__
  dev->window = *((::Window*)hwnd);
#endif

  dev->initExt();
  T_ASSERT_X( errCk(), "OpenGL error" );

  dev->glGenVertexArrays    = (PFNGLGENVERTEXARRAYSPROC)getAddr("glGenVertexArrays");
  dev->glBindVertexArray    = (PFNGLBINDVERTEXARRAYPROC)getAddr("glBindVertexArray");
  dev->glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)getAddr("glDeleteVertexArrays");

  glEnable( GL_DEPTH_TEST );
  glFrontFace( GL_CW );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  dev->glGenVertexArrays(1, &dev->vao);
  dev->glBindVertexArray(dev->vao);

  reset( (AbstractAPI::Device*)dev, hwnd, opt );
  setRenderState( (AbstractAPI::Device*)dev, Tempest::RenderState() );

  T_ASSERT_X( errCk(), "OpenGL error" );
  return (AbstractAPI::Device*)dev;
  }

void Opengl4x::deleteDevice(GraphicsSubsystem::Device *d) const {
  setDevice(d);
  ((ImplDevice*)dev)->glDeleteVertexArrays(1, &dev->vbo);
  Opengl2x::deleteDevice(d);
  }

bool Opengl4x::createContext( Detail::ImplDeviceBase * dev,
                              void *hwnd,
                              const AbstractAPI::Options & ) const {
  (void)hwnd;

#ifdef __WINDOWS__
  int attribs[] =
  {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
    WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };

  typedef HGLRC (GLAPIENTRY *PFNWGLCREATECONTEXTATTRIBSARBPROC)
      (HDC hDC, HGLRC hshareContext,const int *attribList);
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

  HGLRC hRC = Detail::createContext( hDC );
  wglMakeCurrent( hDC, hRC );

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

  dev->hDC = hDC;
  dev->hRC = hRC;

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);

  if( !wglCreateContextAttribsARB ){
    return false;
    }

  hRC = wglCreateContextAttribsARB(dev->hDC, 0, attribs);

  if (!hRC || !wglMakeCurrent(hDC, hRC)) {
    return false;
    }

  dev->hDC = hDC;
  dev->hRC = hRC;

  int major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  Log() << "OpenGL render context information:";
  Log() << "  Renderer       : " << (const char*)glGetString(GL_RENDERER);
  Log() << "  Vendor         : " << (const char*)glGetString(GL_VENDOR);
  Log() << "  Version        : " << (const char*)glGetString(GL_VERSION);
  Log() << "  GLSL version   : " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  Log() << "  OpenGL version : " << major <<"." << minor;

  if( !Detail::initGLProc() ) {
    return 0;
    }
  return 1;
#endif

#ifdef __LINUX__
  dev->dpy = (Display*)LinuxAPI::display();
  Display *dpy = dev->dpy;//FIXME
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo  *vi = glXChooseVisual (dpy, 0, att);
  dev->glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  XFree( vi );

  glXMakeCurrent( dev->dpy, dev->window, dev->glc);

  if( !Detail::initGLProc() ) {
    return 0;
    }
  return 1;
#endif
  return 0;
  }

#endif
