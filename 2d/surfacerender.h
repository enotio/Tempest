#ifndef SURFACERENDER_H
#define SURFACERENDER_H

#include <Tempest/Painter>
#include <Tempest/Color>
#include <Tempest/VertexBuffer>
#include <Tempest/VertexDeclaration>
#include <Tempest/RenderState>
#include <Tempest/Sprite>

#include <vector>

namespace Tempest{

class VertexShaderHolder;
class FragmentShaderHolder;

class VertexBufferHolder;
class IndexBufferHolder;

class Device;
class Surface;

class Font;

class SurfaceRender {
  public:
    SurfaceRender( Tempest::VertexShaderHolder   & vs,
                   Tempest::FragmentShaderHolder & fs );

    void buildVbo( Widget &surf,
                   Tempest::VertexBufferHolder & vbHolder,
                   Tempest::IndexBufferHolder  & ibHolder,
                   Tempest::SpritesHolder & sp );

    template< class T, class F >
    void buildVbo( T &surf,
                   F func,
                   int w,
                   int h,
                   Tempest::VertexBufferHolder & vbHolder,
                   Tempest::IndexBufferHolder  & /*ibHolder*/,
                   Tempest::SpritesHolder & sp ){
      PaintDev p(*this, sstk, sp, 0, 0, w,h);
      PaintEvent e(p,0);

      invW =  2.0f/w;
      invH = -2.0f/h;

      invTw = 1.0f;
      invTh = 1.0f;

      cpuGm.clear();
      blocks.clear();

      func(surf, e);

      vbo = Tempest::VertexBuffer<Vertex>();
      vbo = vbHolder.load( cpuGm );

      sp.flush();

      if( !vdecl.isValid() )
        vdecl = Tempest::VertexDeclaration( vbHolder.device(), decl() );
      }

    void renderTo( Tempest::Device& dev );

  protected:
    struct RState{
      BlendMode bm;
      Tempest::Color cl;
      bool flip[2];
      };

    struct PaintDev;
    struct TextEngine:public Tempest::PaintTextEngine{
      TextEngine( PaintDev & p );

      void setFont(const Font &f );
      void drawText( int x, int y, int w, int h, const std::wstring &,
                     int align = Tempest::NoAlign );
      void drawText( int x, int y, int w, int h, const std::string &,
                     int align = Tempest::NoAlign );
      const Font::Letter& letter( const Font &f, wchar_t c );
      PaintDev & p;
      Tempest::Font font;

      template< class T >
      void dText( int x, int y, int w, int h, const T &,
                  int align );
      };

    struct PaintDev: public PainterDevice {
      PaintDev( SurfaceRender &s, std::vector<RState>& sstk,
                Tempest::SpritesHolder & sp,
                int rx, int ry, int rw, int rh );

      virtual void setTexture( const Tempest::Texture2d & t );
      virtual void setTexture( const Tempest::Sprite & t );
      virtual void unsetTexture();

      void setColor(float, float, float, float);
      void setFlip( bool h, bool v );

      virtual void quad( int x, int y, int  w, int  h,
                         int tx, int ty, int tw, int th );
      virtual void line( int x, int y, int x2, int y2);

      virtual void setBlendMode( BlendMode m );

      virtual PaintTextEngine& textEngine();

      virtual void setNullState();
      virtual void pushState();
      virtual void popState();

      private:
        SurfaceRender &surf;
        std::vector<RState>& sstk;

        TextEngine te;
        Tempest::SpritesHolder & sp;

      friend class TextEngine;
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

    //Tempest::Device &dev;
    Tempest::VertexShaderHolder   & vsH;
    Tempest::FragmentShaderHolder & fsH;

    struct Block{
      size_t begin, end;
      bool   isLine;

      RState  state;
      Sprite     curTex;
      const Texture2d* curTex2d;
      };

    std::vector<RState> sstk;
    std::vector<Block>  blocks;

    std::vector<Vertex> cpuGm;
    Tempest::VertexBuffer<Vertex> vbo;
    Tempest::VertexDeclaration    vdecl;

    Tempest::VertexShader   vs[2];
    Tempest::FragmentShader fs[2];
    float invW, invH, invTw, invTh;

    RState state;
    Sprite     curTex;
    const Texture2d* curTex2d;

    Tempest::RenderState rstate[4];
    bool dpos;

    void loadShader();
    void updateBackBlock(bool isLine);
  };

}

#endif // SURFACERENDER_H
