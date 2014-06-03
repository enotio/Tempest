#include "tessshader.h"

#include <Tempest/Device>

using namespace Tempest;

TessShader::TessShader():data( TessShaderHolder::ImplManip(0) ){

  }

bool TessShader::isValid() const {
  return !data.isNull() && data.const_value();
  }

TessShader::TessShader( AbstractHolder< Tempest::TessShader,
                        AbstractShadingLang::TessShader >& h )
             :data( h.makeManip() ) {

  }
