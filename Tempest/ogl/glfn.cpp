#include "glfn.h"

#ifdef __WIN32
#include "windows.h"
#endif

namespace Tempest {
namespace GLProc {
#ifndef __ANDROID__
void ( GLAPIENTRY *glGenFramebuffers)(GLsizei n, const GLuint* framebuffers) = 0;
void ( GLAPIENTRY *glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers) = 0;

void ( GLAPIENTRY *glGenRenderbuffers)(GLsizei n, const GLuint* framebuffers) = 0;
bool ( GLAPIENTRY *glIsRenderbuffer) (GLuint renderbuffer) = 0;
void ( GLAPIENTRY *glDeleteRenderbuffers)(GLsizei n, const GLuint* framebuffers) = 0;

void ( GLAPIENTRY *glBindBuffer)(GLenum target, GLuint buffer) = 0;
void ( GLAPIENTRY *glBindRenderbuffer)(GLenum target, GLuint renderbuffer) = 0;
void ( GLAPIENTRY *glGenBuffers) (GLsizei n, GLuint* buffers) = 0;
bool ( GLAPIENTRY *glIsBuffer) (GLuint buffer) = 0;
void ( GLAPIENTRY *glDeleteBuffers) (GLsizei n, const GLuint* buffers) = 0;
void ( GLAPIENTRY *glBufferData) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) = 0;
void ( GLAPIENTRY *glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) = 0;

void ( GLAPIENTRY *glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = 0;
void ( GLAPIENTRY *glBindFramebuffer)(GLenum target, GLuint framebuffer) = 0;
void ( GLAPIENTRY *glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = 0;
void ( GLAPIENTRY *glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = 0;
GLenum ( GLAPIENTRY *glCheckFramebufferStatus) (GLenum target) = 0;
void ( GLAPIENTRY *glDrawBuffers)( GLsizei n, const GLenum *bufs) = 0;
void ( GLAPIENTRY *glGenerateMipmap) (GLenum target) = 0;
#ifdef __WIN32__
void ( GLAPIENTRY *glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat,
                                       GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) = 0;
void ( GLAPIENTRY *glActiveTexture) (GLenum texture) = 0;
void ( GLAPIENTRY *glTexImage3D)(	GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth,
                              GLint border, GLenum format, GLenum type, const GLvoid * data) = 0;
#endif
void ( GLAPIENTRY *glEnableVertexAttribArray) (GLuint index) = 0;
void ( GLAPIENTRY *glDisableVertexAttribArray) (GLuint index) = 0;
void ( GLAPIENTRY *glVertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) = 0;

GLuint ( GLAPIENTRY *glCreateShader) (GLenum type) = 0;
void   ( GLAPIENTRY *glDeleteShader) (GLuint shader) = 0;
void   ( GLAPIENTRY *glShaderSource) (GLuint shader, GLsizei count, const GLchar** string, const GLint* length) = 0;
void   ( GLAPIENTRY *glCompileShader)(GLuint shader) = 0;

GLint  ( GLAPIENTRY *glGetUniformLocation) (GLuint program, const GLchar* name) = 0;
void   ( GLAPIENTRY *glGetShaderiv) (GLuint shader, GLenum pname, GLint* params) = 0;
void   ( GLAPIENTRY *glGetShaderInfoLog) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) = 0;
void   ( GLAPIENTRY *glUseProgram) (GLuint program) = 0;
bool   ( GLAPIENTRY *glIsShader)  (GLuint shader) = 0;
bool   ( GLAPIENTRY *glIsProgram) (GLuint program) = 0;
GLuint ( GLAPIENTRY *glCreateProgram) (void) = 0;
void   ( GLAPIENTRY *glDeleteProgram) (GLuint program) = 0;
void   ( GLAPIENTRY *glAttachShader)  (GLuint program, GLuint shader) = 0;
void   ( GLAPIENTRY *glBindAttribLocation) (GLuint program, GLuint index, const GLchar* name) = 0;
void   ( GLAPIENTRY *glLinkProgram)        (GLuint program) = 0;
void   ( GLAPIENTRY *glGetProgramiv) (GLuint program, GLenum pname, GLint* params) = 0;
void   ( GLAPIENTRY *glGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) = 0;

void   ( GLAPIENTRY *glUniform1iv) (GLint location, GLsizei count, const GLint* v) = 0;
void   ( GLAPIENTRY *glUniform1fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   ( GLAPIENTRY *glUniform2fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   ( GLAPIENTRY *glUniform3fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   ( GLAPIENTRY *glUniform4fv) (GLint location, GLsizei count, const GLfloat* v) = 0;

void   ( GLAPIENTRY *glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

#endif
}
}

using namespace Tempest;
using namespace Tempest::GLProc;

Detail::GLContext Detail::createContext(void *hdc) {
#ifdef __WIN32
  return wglCreateContext( (HDC)hdc );
#endif

#ifdef __ANDROID__
  (void)hdc;
  //return eglCreateContext( hdc );
#endif
  return 0;
  }

static void* getAddr( const char* name ){
#ifdef __WIN32
  return (void*)wglGetProcAddress(name);
#endif

#ifdef __ANDROID__
  return (void*)eglGetProcAddress( name );
#endif

#if defined(__linux__) && !defined(__ANDROID__)
  return (void*)glXGetProcAddress( (const GLubyte*)name );
#endif
  return 0;
  }

template< class T >
static bool getP( T& t, const char* name ){
  t = (T)getAddr(name);
  return t;
  }

#define get(X) getP(X, #X)

bool Detail::initGLProc() {
  bool ok =
#ifndef __ANDROID__
      get(glGenFramebuffers) &
      get(glDeleteFramebuffers) &
      get(glGenRenderbuffers) &
      get(glIsRenderbuffer) &
      get(glDeleteRenderbuffers) &
      get(glBindRenderbuffer) &

      get(glBindBuffer) &
      get(glGenBuffers) &
      get(glIsBuffer) &
      get(glDeleteBuffers) &
      get(glBufferData) &
      get(glBufferSubData) &

      get(glRenderbufferStorage) &
      get(glBindFramebuffer) &
      get(glFramebufferTexture2D) &
      get(glFramebufferRenderbuffer) &
      get(glCheckFramebufferStatus) &
      get(glDrawBuffers) &
      get(glGenerateMipmap) &

    #ifdef __WIN32__
      get(glCompressedTexImage2D) &
      get(glActiveTexture) &
      get(glTexImage3D) &
    #endif

      get(glEnableVertexAttribArray) &
      get(glDisableVertexAttribArray) &
      get(glVertexAttribPointer) &

      get(glCreateShader) &
      get(glDeleteShader) &
      get(glShaderSource) &
      get(glCompileShader) &
      get(glGetUniformLocation) &
      get(glGetShaderiv) &
      get(glGetShaderInfoLog) &
      get(glUseProgram) &
      get(glIsShader) &
      get(glIsProgram) &
      get(glCreateProgram) &
      get(glDeleteProgram) &
      get(glAttachShader) &
      get(glBindAttribLocation) &
      get(glLinkProgram) &
      get(glGetProgramiv) &
      get(glGetProgramInfoLog) &

      get(glUniform1iv)&
      get(glUniform1fv)&
      get(glUniform2fv)&
      get(glUniform3fv)&
      get(glUniform4fv)&
      get(glUniformMatrix4fv)&

#endif
      true;

  return ok;
  }

