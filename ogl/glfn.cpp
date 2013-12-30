#include "glfn.h"

#ifdef __WIN32
#include "windows.h"
#endif

namespace Tempest {
namespace GLProc {
#ifndef __ANDROID__
void (*glGenFramebuffers)(GLsizei n, const GLuint* framebuffers) = 0;
void (*glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers) = 0;

void (*glGenRenderbuffers)(GLsizei n, const GLuint* framebuffers) = 0;
bool (*glIsRenderbuffer) (GLuint renderbuffer) = 0;
void (*glDeleteRenderbuffers)(GLsizei n, const GLuint* framebuffers) = 0;

void (*glBindBuffer)(GLenum target, GLuint buffer) = 0;
void (*glBindRenderbuffer)(GLenum target, GLuint renderbuffer) = 0;
void (*glGenBuffers) (GLsizei n, GLuint* buffers) = 0;
bool (*glIsBuffer) (GLuint buffer) = 0;
void (*glDeleteBuffers) (GLsizei n, const GLuint* buffers) = 0;
void (*glBufferData) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) = 0;
void (*glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) = 0;

void (*glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = 0;
void (*glBindFramebuffer)(GLenum target, GLuint framebuffer) = 0;
void (*glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = 0;
void (*glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = 0;
GLenum (*glCheckFramebufferStatus) (GLenum target) = 0;
void (*glGenerateMipmap) (GLenum target) = 0;
void (*glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat,
                                       GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) = 0;
void (*glActiveTexture) (GLenum texture) = 0;
void (*glEnableVertexAttribArray) (GLuint index) = 0;
void (*glDisableVertexAttribArray) (GLuint index) = 0;
void (*glVertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) = 0;

void (*glTexImage3D)(	GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth,
                              GLint border, GLenum format, GLenum type, const GLvoid * data) = 0;

GLuint (*glCreateShader) (GLenum type) = 0;
void   (*glDeleteShader) (GLuint shader) = 0;
void   (*glShaderSource) (GLuint shader, GLsizei count, const GLchar** string, const GLint* length) = 0;
void   (*glCompileShader)(GLuint shader) = 0;

GLint  (*glGetUniformLocation) (GLuint program, const GLchar* name) = 0;
void   (*glGetShaderiv) (GLuint shader, GLenum pname, GLint* params) = 0;
void   (*glGetShaderInfoLog) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) = 0;
void   (*glUseProgram) (GLuint program) = 0;
bool   (*glIsShader)  (GLuint shader) = 0;
bool   (*glIsProgram) (GLuint program) = 0;
GLuint (*glCreateProgram) (void) = 0;
void   (*glDeleteProgram) (GLuint program) = 0;
void   (*glAttachShader)  (GLuint program, GLuint shader) = 0;
void   (*glBindAttribLocation) (GLuint program, GLuint index, const GLchar* name) = 0;
void   (*glLinkProgram)        (GLuint program) = 0;
void   (*glGetProgramiv) (GLuint program, GLenum pname, GLint* params) = 0;
void   (*glGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) = 0;

void   (*glUniform1iv) (GLint location, GLsizei count, const GLint* v) = 0;
void   (*glUniform1fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   (*glUniform2fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   (*glUniform3fv) (GLint location, GLsizei count, const GLfloat* v) = 0;
void   (*glUniform4fv) (GLint location, GLsizei count, const GLfloat* v) = 0;

void   (*glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;

#endif
}
}

using namespace Tempest;
using namespace Tempest::GLProc;

static void* getAddr( const char* name ){
#ifdef __WIN32
  return (void*)wglGetProcAddress(name);
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
      get(glGenerateMipmap) &
      get(glCompressedTexImage2D) &
      get(glActiveTexture) &

      get(glEnableVertexAttribArray) &
      get(glDisableVertexAttribArray) &
      get(glVertexAttribPointer) &

      get(glTexImage3D) &

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

