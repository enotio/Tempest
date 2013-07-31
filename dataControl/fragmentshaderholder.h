#ifndef FRAGMENTSHADERHOLDER_H
#define FRAGMENTSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class FragmentShader;
class Device;

struct FragmentShaderHolder: ShaderHolder< FragmentShader,
                                           AbstractShadingLang::FragmentShader,
                                           AbstractShadingLang::Fragment > {
  FragmentShaderHolder( Device& d ):ShaderHolder<
                                    FragmentShader,
                                    AbstractShadingLang::FragmentShader,
                                    AbstractShadingLang::Fragment >(d){}
  };
}

#endif // FRAGMENTSHADERHOLDER_H
