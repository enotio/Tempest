#ifndef FRAGMENTSHADERHOLDER_H
#define FRAGMENTSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class FragmentShader;
class Device;

struct FragmentShaderHolder: ShaderHolder< FragmentShader,
                                           AbstractAPI::FragmentShader,
                                           AbstractAPI::Fragment > {
  FragmentShaderHolder( Device& d ):ShaderHolder<
                                    FragmentShader,
                                    AbstractAPI::FragmentShader,
                                    AbstractAPI::Fragment >(d){}
  };
}

#endif // FRAGMENTSHADERHOLDER_H
