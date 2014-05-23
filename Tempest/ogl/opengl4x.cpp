#include "opengl4x.h"

#ifndef __ANDROID__
#include <GL/gl.h>

#ifdef __WIN32
#include <windows.h>
#include "glfn.h"
#endif

using namespace Tempest;

OpenGL4x::OpenGL4x() {

  }

AbstractAPI::Device *OpenGL4x::createDevice( void *hwnd,
                                             const AbstractAPI::Options &opt ) const {
#ifdef __WIN32
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

  //dev->hDC = hDC;
  //dev->hRC = hRC;

  //dev->wglSwapInterval = 0;

  if( !Detail::initGLProc() ) {
    return 0;
    }
#endif

  return 0;
  }

void OpenGL4x::deleteDevice(Opengl2x::Device *d) const {
  }

#endif
