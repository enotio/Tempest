#include "gltypes.h"

Tempest::Detail::GLTexture::GLTexture() {
  id = 0;
  depthId = 0;
  w = 0;
  h = 0;
  min = mag = GL_NEAREST;
  mips = false;

  format = 0;
  anisotropyLevel = 1;

  fbo    = 0;  
  fboTg = 0;
  }
/*
GLuint Tempest::Detail::FBOPool::get(GLuint texture) {
  for( size_t i=0; i<data.size(); ++i ){
    if( data[i].w==w &&
        data[i].h==h &&
        data[i].frm==frm )
      return data[i].fbo;
    }

  Chunk c;
  c.w = w;
  c.h = h;
  c.frm = frm;

  glGenFramebuffers( 1, &c.fbo);
  data.push_back(c);

  return c.fbo;
  }
*/
