#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/VertexShaderHolder>

#include "shader.h"

namespace Tempest{

class VertexShader : public Shader {
  public:
    VertexShader();

    bool isValid() const;
  private:
    VertexShader( AbstractHolder< Tempest::VertexShader,
                                  AbstractAPI::VertexShader >& h );

    Detail::Ptr< AbstractAPI::VertexShader*,
                 VertexShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  template< class Shader, class APIDescriptor, AbstractAPI::ShaderType >
  friend class ShaderHolder;
  };

}

#endif // VERTEXSHADER_H
