#ifndef EVALSHADERHOLDER_H
#define EVALSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class EvalShader;
class DeviceSM5;

struct EvalShaderHolder: ShaderHolder< EvalShader,
                                       AbstractAPI::EvalShader,
                                       AbstractAPI::Eval > {
  EvalShaderHolder( DeviceSM5& d ):ShaderHolder<
                                       EvalShader,
                                       AbstractAPI::EvalShader,
                                       AbstractAPI::Eval >(d){}
  };
}

#endif // EVALSHADERHOLDER_H
