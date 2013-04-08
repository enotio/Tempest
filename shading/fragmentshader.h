#ifndef FRAGMENTSHADER_H
#define FRAGMENTSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/FragmentShaderHolder>

#include "shaderinput.h"

namespace Tempest{

class FragmentShader {
  public:
    FragmentShader();

  private:
    FragmentShader( AbstractHolder< Tempest::FragmentShader,
                                    AbstractShadingLang::FragmentShader >& h );
    Detail::Ptr< AbstractShadingLang::FragmentShader*,
                 FragmentShaderHolder::ImplManip > data;
    ShaderInput input;


  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  friend class FragmentShaderHolder;
  friend class Device;
  };

}

#endif // FRAGMENTSHADER_H
