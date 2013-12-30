#include "vertexshader.h"

#include <Tempest/Device>

using namespace Tempest;

VertexShader::VertexShader():data( VertexShaderHolder::ImplManip(0) ){

  }

bool VertexShader::isValid() const {
  return !data.isNull() && data.const_value();
  }

VertexShader::VertexShader( AbstractHolder< Tempest::VertexShader,
                            AbstractShadingLang::VertexShader >& h )
             :data( h.makeManip() ) {

  }
