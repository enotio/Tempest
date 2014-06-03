#ifndef EVALSHADERHOLDER_H
#define EVALSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class EvalShader;
class Device;

struct EvalShaderHolder: ShaderHolder< EvalShader,
                                       AbstractAPI::EvalShader,
                                       AbstractAPI::Eval > {
  EvalShaderHolder( Device& d ):ShaderHolder<
                                       EvalShader,
                                       AbstractAPI::EvalShader,
                                       AbstractAPI::Eval >(d){}
  };
}

#endif // EVALSHADERHOLDER_H
