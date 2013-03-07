#include "fragmentshader.h"

using namespace Tempest;

FragmentShader::FragmentShader():data( FragmentShaderHolder::ImplManip(0) ){

  }

FragmentShader::FragmentShader( AbstractHolder< Tempest::FragmentShader,
                                  AbstractShadingLang::FragmentShader >& h )
             :data( h.makeManip() ) {

  }
