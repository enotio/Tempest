#ifndef TESSSHADERHOLDER_H
#define TESSSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class TessShader;
class DeviceSM5;

struct TessShaderHolder: ShaderHolder< TessShader,
                                       AbstractAPI::TessShader,
                                       AbstractAPI::Tess > {
  TessShaderHolder( DeviceSM5& d ):ShaderHolder<
                                       TessShader,
                                       AbstractAPI::TessShader,
                                       AbstractAPI::Tess >(d){}
  };
}

#endif // TESSSHADERHOLDER_H
