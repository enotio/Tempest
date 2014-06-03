#ifndef TESSSHADERHOLDER_H
#define TESSSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class TessShader;
class Device;

struct TessShaderHolder: ShaderHolder< TessShader,
                                       AbstractAPI::TessShader,
                                       AbstractAPI::Tess > {
  TessShaderHolder( Device& d ):ShaderHolder<
                                       TessShader,
                                       AbstractAPI::TessShader,
                                       AbstractAPI::Tess >(d){}
  };
}

#endif // TESSSHADERHOLDER_H
