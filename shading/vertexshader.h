#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/VertexShaderHolder>

#include "shaderinput.h"

namespace Tempest{

class VertexShader {
  public:
    VertexShader();

  private:
    VertexShader( AbstractHolder< Tempest::VertexShader,
                                  AbstractShadingLang::VertexShader >& h );

    Detail::Ptr< AbstractShadingLang::VertexShader*,
                 VertexShaderHolder::ImplManip > data;

    ShaderInput input;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  friend class VertexShaderHolder;
  friend class Device;
  };

}

#endif // VERTEXSHADER_H
