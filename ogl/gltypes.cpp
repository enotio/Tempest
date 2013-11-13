#include "gltypes.h"

Tempest::Detail::GLTexture::GLTexture() {
  id = 0;
  depthId = 0;
  w = 0;
  h = 0;
  min    = mag    = GL_NEAREST;
  clampU = clampV = GL_REPEAT;
  clampW = GL_REPEAT;
  mips = false;

  format = 0;
  anisotropyLevel = 1;

  pixelFormat  = GL_UNSIGNED_BYTE;
  isInitalized = true;

  fboHash = -1;
  }
