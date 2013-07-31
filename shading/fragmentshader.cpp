#include "fragmentshader.h"

#include <Tempest/Device>

using namespace Tempest;

FragmentShader::FragmentShader():data( FragmentShaderHolder::ImplManip(0) ){

  }

bool FragmentShader::isValid() const {
  return !data.isNull() && data.const_value();
  }

FragmentShader::FragmentShader( AbstractHolder< Tempest::FragmentShader,
                                  AbstractShadingLang::FragmentShader >& h )
             :data( h.makeManip() ) {

  }
