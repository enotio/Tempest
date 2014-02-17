#ifndef VERTEXSHADERHOLDER_H
#define VERTEXSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class VertexShader;
class Device;

struct VertexShaderHolder: ShaderHolder< VertexShader,
                                         AbstractAPI::VertexShader,
                                         AbstractAPI::Vertex > {
  VertexShaderHolder( Device& d ):ShaderHolder<
                                    VertexShader,
                                    AbstractAPI::VertexShader,
                                    AbstractAPI::Vertex >(d){}
  };
}

#endif // VERTEXSHADERHOLDER_H
