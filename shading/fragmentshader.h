#ifndef FRAGMENTSHADER_H
#define FRAGMENTSHADER_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/FragmentShaderHolder>

namespace Tempest{

class FragmentShader {
  public:
    FragmentShader();

  private:
    FragmentShader( AbstractHolder< Tempest::FragmentShader,
                                    AbstractShadingLang::FragmentShader >& h );
    Detail::Ptr< AbstractShadingLang::FragmentShader*,
                 FragmentShaderHolder::ImplManip > data;

  friend class AbstractShadingLang;

  template< class Data, class APIDescriptor >
  friend class AbstractHolderWithLoad;

  friend class FragmentShaderHolder;
  };

}

#endif // FRAGMENTSHADER_H
