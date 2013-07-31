#ifndef VERTEXSHADERHOLDER_H
#define VERTEXSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/ShaderHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class VertexShader;
class Device;

struct VertexShaderHolder: ShaderHolder< VertexShader,
                                           AbstractShadingLang::VertexShader,
                                           AbstractShadingLang::Vertex > {
  VertexShaderHolder( Device& d ):ShaderHolder<
                                    VertexShader,
                                    AbstractShadingLang::VertexShader,
                                    AbstractShadingLang::Vertex >(d){}
  };
}

#endif // VERTEXSHADERHOLDER_H
