#include "evalshader.h"

#include <Tempest/Device>

using namespace Tempest;

EvalShader::EvalShader():data( EvalShaderHolder::ImplManip(0) ){

  }

bool EvalShader::isValid() const {
  return !data.isNull() && data.const_value();
  }

EvalShader::EvalShader( AbstractHolder< Tempest::EvalShader,
                        AbstractShadingLang::EvalShader >& h )
             :data( h.makeManip() ) {

  }
