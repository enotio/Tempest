#include "openglbase.h"

#include <Tempest/Platform>
#include "gltypes.h"

//#define OGL_DEBUG
#ifdef __ANDROID__
//GL_HALF_FLOAT -> GL_HALF_FLOAT_OES -> 0x8D61
#define GL_HALF_FLOAT 0x8D61

//#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#else
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include "glfn.h"
#include <GL/gl.h>
#endif

static const GLubyte* vstr( const GLubyte* v ){
  if( v )
    return v; else
    return (GLubyte*)"";
  }

using namespace Tempest;

OpenGLBase::OpenGLBase(){
  }

bool OpenGLBase::errCk() const {
#ifdef OGL_DEBUG
  GLenum err = glGetError();
  bool ok = true;

  while( err!=GL_NO_ERROR ){
    const char* glErrorDesc = Detail::ImplDeviceBase::glErrorDesc(err);
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
  return ok;
#endif
  return true;
  }

void OpenGLBase::setClearDepth(float z) const {
#ifdef __ANDROID__
  glClearDepthf( z );
#else
  glClearDepth( z );
#endif
  }

std::string OpenGLBase::vendor( AbstractAPI::Device* d ) const {
  if( !setDevice(d) ) return "";

  const GLubyte *s = vstr(glGetString(GL_VENDOR));
  return (const char*)s;
  }

std::string OpenGLBase::renderer(AbstractAPI::Device *d) const {
  if( !setDevice(d) ) return "";

  const GLubyte *s = vstr(glGetString(GL_RENDERER));
  return (const char*)s;
  }
