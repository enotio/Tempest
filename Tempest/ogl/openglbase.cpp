#include "openglbase.h"

#include <Tempest/Platform>
#include <Tempest/Log>
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
#ifdef __OSX__
#include <OpenGL/gl.h>
#elif defined(__IOS__)
#include <OpenGLES/ES2/gl.h>
#else
#include <GL/gl.h>
#endif
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
    Log::e("[OpenGL]: ",(glErrorDesc ? glErrorDesc : "")," ",(void*)err);
    err = glGetError();
    ok = false;
    }
  return ok;
#endif
  return true;
  }

void OpenGLBase::setClearDepth(float z) const {
#ifdef __MOBILE_PLATFORM__
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
