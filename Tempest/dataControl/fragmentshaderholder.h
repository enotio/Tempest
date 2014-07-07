#ifndef FRAGMENTSHADERHOLDER_H
#define FRAGMENTSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class FragmentShader;
class Device;

class FragmentShaderHolder:public ShaderHolder< FragmentShader,
                                                 AbstractAPI::FragmentShader,
                                                 AbstractAPI::Fragment > {
  public:
  FragmentShaderHolder( Device& d ):ShaderHolder<
                                    FragmentShader,
                                    AbstractAPI::FragmentShader,
                                    AbstractAPI::Fragment >(d){}
  };
}

#endif // FRAGMENTSHADERHOLDER_H
