#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/VertexShaderHolder>

namespace Tempest{

class VertexShader {
  public:
    VertexShader();

  private:
    VertexShader( AbstractHolder< Tempest::VertexShader,
                                  AbstractShadingLang::VertexShader >& h );

    Detail::Ptr< AbstractShadingLang::VertexShader*,
                 VertexShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  friend class VertexShaderHolder;
  };

}

#endif // VERTEXSHADER_H
