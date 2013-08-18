#ifndef SURFACE_H
#define SURFACE_H

#include <Tempest/Widget>
#include <Tempest/Painter>
#include <Tempest/VertexDeclaration>
#include <Tempest/VertexBuffer>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/Color>
#include <Tempest/RenderState>

namespace Tempest{

class Device;
class Sprite;

class VertexBufferHolder;
class IndexBufferHolder;

class VertexShaderHolder;
class FragmentShaderHolder;

class Surface : public Widget {
  public:
    Surface( Tempest::VertexShaderHolder   & vs,
             Tempest::FragmentShaderHolder & fs );

    void buildVbo( Tempest::VertexBufferHolder & vbHolder,
                   Tempest::IndexBufferHolder  & ibHolder );
    void renderTo( Device& dev );

  protected:
    struct RState{
      BlendMode bm;
      Tempest::Color cl;
      };

    struct PaintDev: public PainterDevice {
      PaintDev( Surface &s, std::vector<RState>& sstk );

      virtual void setTexture( const Texture & t );
      virtual void setTexture( const Tempest::Texture2d & t );
      virtual void setTexture( const Tempest::Sprite & t );
      virtual void unsetTexture();

      virtual void quad( int x, int y, int  w, int  h,
                         int tx, int ty, int tw, int th );
      virtual void line( int x, int y, int x2, int y2);

      virtual void setBlendMode( BlendMode m );

      //virtual PaintTextEngine& textEngine();

      virtual void setNullState();
      virtual void pushState();
      virtual void popState();

      private:
        Surface &surf;
        std::vector<RState>& sstk;
      };

    virtual void quad( int x, int y, int  w, int  h,
                       int tx, int ty, int tw, int th );
    virtual void line( int x, int y, int x2, int y2);

    const Tempest::VertexDeclaration::Declarator &decl();
    static const Tempest::VertexDeclaration::Declarator declImpl();

    struct Vertex{
      float x,y;
      float u,v;
      float color[4];
      };

    Tempest::VertexShaderHolder   & vsH;
    Tempest::FragmentShaderHolder & fsH;

    struct Block{
      size_t begin, end;
      bool   isLine;

      RState  state;
      const Sprite    * curTex;
      const Texture2d * curTex2d;
      };

    std::vector<RState> sstk;
    std::vector<Block>  blocks;

    std::vector<Vertex> cpuGm;
    Tempest::VertexBuffer<Vertex> vbo;
    Tempest::VertexDeclaration    vdecl;

    Tempest::VertexShader   vs;
    Tempest::FragmentShader fs;
    float invW, invH, invTw, invTh;

    RState state;
    const Sprite    * curTex;
    const Texture2d * curTex2d;

    Tempest::RenderState rstate[4];

    void loadShader();
    void updateBackBlock(bool isLine);
  };

}

#endif // SURFACE_H
