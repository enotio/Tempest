#ifndef SURFACERENDER_H
#define SURFACERENDER_H

#include <Tempest/Painter>
#include <Tempest/Color>
#include <Tempest/VertexBuffer>
#include <Tempest/VertexDeclaration>
#include <Tempest/RenderState>
#include <Tempest/Sprite>
#include <Tempest/UniformDeclaration>

#include <vector>

namespace Tempest{

class ShaderProgramHolder;

class VertexBufferHolder;
class IndexBufferHolder;

class Device;
class Surface;

class Font;

class SurfaceRender {
  public:
    SurfaceRender( Tempest::ShaderProgramHolder& sh );

    void clearVbo();

    void buildWindowVbo( Window &window,
                         Tempest::VertexBufferHolder & vbHolder,
                         Tempest::IndexBufferHolder  & ibHolder,
                         Tempest::SpritesHolder & spHolder,
                         bool clrVbo   = true,
                         bool flushVbo = true);

    void buildVbo( Widget &surf,
                   Tempest::VertexBufferHolder & vbHolder,
                   Tempest::IndexBufferHolder  & ibHolder,
                   Tempest::SpritesHolder & sp,
                   bool clrVbo   = true,
                   bool flushVbo = true,
                   int pass      = 0 );

    template< class T, class F >
    void buildVbo( T &surf,
                   F func,
                   int w,
                   int h,
                   Tempest::VertexBufferHolder & vbHolder,
                   Tempest::IndexBufferHolder  & /*ibHolder*/,
                   Tempest::SpritesHolder & sp,
                   bool clrVbo   = true,
                   bool flushVbo = true,
                   int pass      = 0 ){
      surf.nToUpdate = false;

      PaintDev p(*this, sstk, sp, 0, 0, w,h);
      PaintEvent e(p,pass);

      invW =  2.0f/w;
      invH = -2.0f/h;

      invTw = 1.0f;
      invTh = 1.0f;

      if( clrVbo ){
        cpuGm.clear();
        blocks.clear();
        }

      func(surf, e);

      if( flushVbo ){
        vbo = Tempest::VertexBuffer<Vertex>();
        vbo = vbHolder.load( cpuGm, AbstractAPI::BF_NoReadback );
        }

      sp.flush();
      }

    void render( Tempest::Device& dev ) const;

  protected:
    struct RState{
      BlendMode bm;
      Tempest::Color cl;
      bool flip[2];

      Sprite     curTex;
      const Texture2d* curTex2d;
      };

    struct PaintDev;
    class TextEngine:public Tempest::PaintTextEngine{
      public:
      TextEngine( PaintDev & p );

      void pushState();
      void popState();
      void setNullState();

      void setFont(const Font &f );
      Font font() const;
      void drawText( int x, int y, int w, int h,
                     const char16_t* ,
                     int align = Tempest::NoAlign );
      void drawText( int x, int y, int w, int h,
                     const char*,
                     int align = Tempest::NoAlign );
      const Font::Letter& letter( const Font &f, wchar_t c );
      PaintDev & p;
      Tempest::Font fnt;
      std::vector<Tempest::Font> state_stk;

      template< class T >
      void dText( int x, int y, int w, int h, const T* str,
                  int align );
      };

    struct PaintDev: public PainterDevice {
      PaintDev( SurfaceRender &s, std::vector<RState>& sstk,
                Tempest::SpritesHolder & sp,
                int rx, int ry, int rw, int rh );

      virtual void setTexture( const Tempest::Texture2d & t );
      virtual void setTexture( const Tempest::Sprite & t );
      virtual void unsetTexture();

      Color color() const;
      void setColor( Color& cl );
      void setColor(float, float, float, float);

      void setFlip( bool h, bool v );
      bool isHorizontalFliped() const;
      bool isVerticalFliped() const;

      virtual void quad( int x, int y, int  w, int  h,
                         int tx, int ty, int tw, int th );
      virtual void triangle( int x1, int y1, int u1, int v1,
                             int x2, int y2, int u2, int v2,
                             int x3, int y3, int u3, int v3 );
      virtual void line( int x, int y, int x2, int y2);

      virtual void setBlendMode( BlendMode m );
      virtual BlendMode blendMode() const;

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
    virtual void triangle( int x1, int y1, int u1, int v1,
                           int x2, int y2, int u2, int v2,
                           int x3, int y3, int u3, int v3 );
    virtual void line( int x, int y, int x2, int y2);

    const Tempest::VertexDeclaration::Declarator &decl();
    static const Tempest::VertexDeclaration::Declarator declImpl();

    struct Vertex{
      float x,y;
      float u,v;
      float color[4];
      };

    Tempest::ShaderProgramHolder& shH;

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

    virtual ShaderProgram& shaderForBlock( const Block& b ) const;
    virtual size_t cpuVertexCurrentCount() const;
    Tempest::VertexBuffer<Vertex> vbo;
    Tempest::VertexDeclaration    vdecl;

    mutable Tempest::ShaderProgram  stdShaders[2];
    float invW, invH, invTw, invTh;

  private:
    RState state;

    Tempest::RenderState rstate[4];
    bool dpos;

    void loadShader();
    void updateBackBlock(bool isLine);

    void update( Vertex* v, int count );

    struct UDpos{
      float     dpos[2];
      };

    struct UBO {
      Texture2d brush;
      };
    UniformDeclaration udecl, dposDecl;

    mutable UDpos udpos;
    mutable UBO   uniforms;
  };

}

#endif // SURFACERENDER_H
