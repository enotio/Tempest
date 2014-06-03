#ifndef TESSSHADER_H
#define TESSSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/TessShaderHolder>

#include "shader.h"

namespace Tempest{

class TessShader : public Shader {
  public:
    TessShader();

    bool isValid() const;
  private:
    TessShader( AbstractHolder< Tempest::TessShader,
                                 AbstractAPI::TessShader >& h );

    Detail::Ptr< AbstractAPI::TessShader*,
                 TessShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  template< class Shader, class APIDescriptor, AbstractAPI::ShaderType >
  friend class ShaderHolder;
  };

}

#endif // TESSSHADER_H
