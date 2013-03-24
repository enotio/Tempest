#include "gltypes.h"

Tempest::Detail::GLTexture::GLTexture() {
  id = 0;
  depthId = 0;
  w = 0;
  h = 0;
  min = mag = GL_NEAREST;
  mips = false;

  format = 0;
  }

