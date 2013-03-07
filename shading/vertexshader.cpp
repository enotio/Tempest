#include "vertexshader.h"

using namespace Tempest;

VertexShader::VertexShader():data( VertexShaderHolder::ImplManip(0) ){

  }

VertexShader::VertexShader( AbstractHolder< Tempest::VertexShader,
                            AbstractShadingLang::VertexShader >& h )
             :data( h.makeManip() ) {

  }
