#ifndef EVALSHADER_H
#define EVALSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/EvalShaderHolder>

#include "shader.h"

namespace Tempest{

class EvalShader : public Shader {
  public:
    EvalShader();

    bool isValid() const;
  private:
    EvalShader( AbstractHolder< Tempest::EvalShader,
                                 AbstractAPI::EvalShader >& h );

    Detail::Ptr< AbstractAPI::EvalShader*,
                 EvalShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  template< class Shader, class APIDescriptor, AbstractAPI::ShaderType >
  friend class ShaderHolder;
  };

}

#endif // EVALSHADER_H
