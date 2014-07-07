#ifndef GLFN_H
#define GLFN_H

#include <Tempest/Platform>
#include <cstddef>
#ifdef __ANDROID__
#include <EGL/egl.h>
#endif

#if __LINUX__
#include <GL/gl.h>
#include <GL/glx.h>

#undef Always // in X11/X.h
#undef PSize
#endif

#if defined(__WINDOWS__) || defined(__LINUX__)
#include <GL/gl.h>

#define GL_HALF_FLOAT 0x140B
#define GL_RG16       0x822C

#define GL_CLAMP_TO_BORDER 0x812D
#define GL_MIRRORED_REPEAT 0x8370
#define GL_TEXTURE_WRAP_R  0x8072

#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

#define GL_DEPTH_STENCIL                                  0x84F9
#define GL_DEPTH_COMPONENT16                              0x81A5
#define GL_DEPTH_COMPONENT24                              0x81A6
#define GL_DEPTH_COMPONENT32                              0x81A7

#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894

#define GL_INVALID_FRAMEBUFFER_OPERATION                  0x0506

#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS         0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD

#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41

#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE_3D                     0x806F

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_MAX_VARYING_COMPONENTS 0x8B4B

#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84

#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_VERTEX_SHADER                    0x8B31
#define GL_LINK_STATUS                      0x8B82

#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef char             GLchar;
#endif

namespace Tempest{

namespace Detail {
#ifdef __WINDOWS__
typedef HGLRC GLContext;
#endif

#ifdef __ANDROID__
typedef EGLContext GLContext;
#endif

#ifdef __LINUX__
typedef GLXContext GLContext;
#endif

#ifdef __WINDOWS_PHONE__
typedef void* GLContext;
#endif

  bool initGLProc();
  GLContext createContext( void* hdc );
  }

namespace GLProc{
#if defined(__WIN32__) || (defined(__linux__) && !defined(__ANDROID__) )
extern void ( GLAPIENTRY *glGenFramebuffers)(GLsizei n, const GLuint* framebuffers);
extern void ( GLAPIENTRY *glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);

extern void ( GLAPIENTRY *glGenRenderbuffers)(GLsizei n, const GLuint* framebuffers);
extern bool ( GLAPIENTRY *glIsRenderbuffer) (GLuint renderbuffer);
extern void ( GLAPIENTRY *glDeleteRenderbuffers)(GLsizei n, const GLuint* framebuffers);

extern void ( GLAPIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
extern void ( GLAPIENTRY *glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
extern void ( GLAPIENTRY *glGenBuffers) (GLsizei n, GLuint* buffers);
extern bool ( GLAPIENTRY *glIsBuffer) (GLuint buffer);
extern void ( GLAPIENTRY *glDeleteBuffers) (GLsizei n, const GLuint* buffers);
extern void ( GLAPIENTRY *glBufferData) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
extern void ( GLAPIENTRY *glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);

extern void ( GLAPIENTRY *glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void ( GLAPIENTRY *glBindFramebuffer)(GLenum target, GLuint framebuffer);
extern void ( GLAPIENTRY *glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void ( GLAPIENTRY *glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern GLenum ( GLAPIENTRY *glCheckFramebufferStatus) (GLenum target);
extern void ( GLAPIENTRY *glGenerateMipmap) (GLenum target);
extern void ( GLAPIENTRY *glDrawBuffers)( GLsizei n, const GLenum *bufs);
#ifdef __WIN32__
extern void ( GLAPIENTRY *glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat,
                                       GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
extern void ( GLAPIENTRY *glActiveTexture) (GLenum texture);
extern void ( GLAPIENTRY *glTexImage3D)(	GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth,
                              GLint border, GLenum format, GLenum type, const GLvoid * data);
#endif
extern void ( GLAPIENTRY *glEnableVertexAttribArray) (GLuint index);
extern void ( GLAPIENTRY *glDisableVertexAttribArray) (GLuint index);
extern void ( GLAPIENTRY *glVertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);

extern GLuint ( GLAPIENTRY *glCreateShader) (GLenum type);
extern void   ( GLAPIENTRY *glDeleteShader) (GLuint shader);
extern void   ( GLAPIENTRY *glShaderSource) (GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
extern void   ( GLAPIENTRY *glCompileShader)(GLuint shader);

extern GLint  ( GLAPIENTRY *glGetUniformLocation) (GLuint program, const GLchar* name);
extern void   ( GLAPIENTRY *glGetShaderiv) (GLuint shader, GLenum pname, GLint* params);
extern void   ( GLAPIENTRY *glGetShaderInfoLog) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern void   ( GLAPIENTRY *glUseProgram) (GLuint program);
extern bool   ( GLAPIENTRY *glIsShader)  (GLuint shader);
extern bool   ( GLAPIENTRY *glIsProgram) (GLuint program);
extern GLuint ( GLAPIENTRY *glCreateProgram) (void);
extern void   ( GLAPIENTRY *glDeleteProgram) (GLuint program);
extern void   ( GLAPIENTRY *glAttachShader)  (GLuint program, GLuint shader);
extern void   ( GLAPIENTRY *glBindAttribLocation) (GLuint program, GLuint index, const GLchar* name);
extern void   ( GLAPIENTRY *glLinkProgram)        (GLuint program);
extern void   ( GLAPIENTRY *glGetProgramiv) (GLuint program, GLenum pname, GLint* params);
extern void   ( GLAPIENTRY *glGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);

extern void   ( GLAPIENTRY *glUniform1iv) (GLint location, GLsizei count, const GLint* v);
extern void   ( GLAPIENTRY *glUniform1fv) (GLint location, GLsizei count, const GLfloat* v);
extern void   ( GLAPIENTRY *glUniform2fv) (GLint location, GLsizei count, const GLfloat* v);
extern void   ( GLAPIENTRY *glUniform3fv) (GLint location, GLsizei count, const GLfloat* v);
extern void   ( GLAPIENTRY *glUniform4fv) (GLint location, GLsizei count, const GLfloat* v);

extern void   ( GLAPIENTRY *glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

#endif
}
}

#endif // GLFN_H
