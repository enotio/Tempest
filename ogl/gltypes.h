#ifndef GLTYPES_H
#define GLTYPES_H

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

namespace Tempest{

namespace Detail{
  struct GLTexture{
    GLTexture();

    GLuint id;
    GLuint depthId;

    GLenum min, mag;
    bool mips;

    int w,h;
    GLenum format;
    };

  struct GLBuffer{
    GLuint id;
    char * mappedData;

    unsigned offset, size;
    };

  struct RenderTg{
    static const int maxMRT = 32;

    GLTexture* color[maxMRT];
    int        mip[maxMRT];

    GLTexture* depth;
    int depthMip;
    };

  }
}

#endif // GLTYPES_H
