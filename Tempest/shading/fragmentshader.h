#ifndef FRAGMENTSHADER_H
#define FRAGMENTSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/FragmentShaderHolder>

#include "shader.h"

namespace Tempest{

class FragmentShader : public Shader {
  public:
    FragmentShader();

    bool isValid() const;
  private:
    FragmentShader( AbstractHolder< Tempest::FragmentShader,
                                    AbstractShadingLang::FragmentShader >& h );
    Detail::Ptr< AbstractShadingLang::FragmentShader*,
                 FragmentShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  template< class Shader, class APIDescriptor, AbstractShadingLang::ShaderType >
  friend class ShaderHolder;
  };

}

#endif // FRAGMENTSHADER_H
