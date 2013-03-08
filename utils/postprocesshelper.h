#ifndef POSTPROCESSHELPER_H
#define POSTPROCESSHELPER_H

#include <Tempest/VertexBuffer>

namespace Tempest{

class Device;
class VertexBufferHolder;
class IndexBufferHolder;

class VertexShader;
class FragmentShader;

class PostProcessHelper {
  public:
    PostProcessHelper( Tempest::VertexBufferHolder&  vbo,
                       Tempest::IndexBufferHolder &  ibo );

    virtual void drawFullScreenQuad( Tempest::Device & device,
                                     Tempest::VertexShader   & vs,
                                     Tempest::FragmentShader & fs ) const;

  private:
    Tempest::VertexBufferHolder&  vbo;
    Tempest::IndexBufferHolder &  ibo;

    struct Vertex{
      float x,y;
      float u,v;
      };

    Tempest::VertexBuffer<Vertex> quad;
    Tempest::VertexDeclaration    vdecl;
  };

}

#endif // POSTPROCESSHELPER_H
