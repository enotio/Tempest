#ifndef GLTYPES_H
#define GLTYPES_H

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
//#include <GL/glew.h>
#include "glfn.h"
#include <GL/gl.h>
#endif

#include <vector>
#include <Tempest/Texture2d>

namespace Tempest{

namespace Detail{
  struct GLBuffer{
    GLuint id;
    char * mappedData;

    unsigned offset, size;
    };

  class GLTexture;
  struct RenderTg{
    static const int maxMRT = 32;

    GLTexture* color[maxMRT];
    int        mip[maxMRT];

    GLTexture* depth;
    int depthMip;
    };

  struct GLTexture{
    GLTexture();

    GLuint id;
    GLuint depthId;

    size_t fboHash;

    GLenum min, mag;
    GLenum clampU,clampV, clampW;
    bool mips, compress;

    bool isInitalized;

    int w,h, z;
    float anisotropyLevel;
    GLenum format, pixelFormat;
    };
  }
}

#endif // GLTYPES_H
