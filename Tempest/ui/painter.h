#ifndef PAINTER_H
#define PAINTER_H

#include <Tempest/PaintTextEngine>
#include <Tempest/Utility>
#include <Tempest/Font>
#include <string>

namespace Tempest{

class PaintEvent;
class PainterDevice;
class Texture2d;
class Sprite;

enum BlendMode{
  noBlend = 0,
  alphaBlend,
  addBlend,
  multiplyBlend,

  userDefined = 128,

  lastBlendMode = 32768
  };

class PainterDevice {
  protected:
    PainterDevice();
    virtual ~PainterDevice();

    void setScissor( int x, int y, int w, int h );
    void setScissor( const Rect &r );
    Rect scissor() const;

    virtual void drawRect( int x, int y, int w, int h,
                           int tx, int ty );

    virtual void drawRect( int x, int y, int w, int h,
                           int tx, int ty, int tw, int th );

    virtual void drawRectTailed( int x, int y, int w, int h,
                                 int tx, int ty );
    virtual void drawRectTailed( int x, int y, int w, int h,
                                 int tx, int ty, int tw, int th );

    virtual void drawLine( int x, int y, int x1, int y1 );

    virtual void translate( int dx, int dy );

    virtual void setTexture( const Tempest::Texture2d& ){}
    virtual void setTexture( const Tempest::Sprite& ){}
    virtual void unsetTexture() = 0;

    virtual void  setColor( float , float , float , float ){}
    virtual void  setColor( Color& ){}
    virtual Color color() const{ return Color(1); }

    virtual void setFlip( bool /*h*/, bool /*v*/ ){}
    virtual bool isHorizontalFliped() const{ return false; }
    virtual bool isVerticalFliped() const{ return false; }

    virtual void quad( int x, int y, int  w, int  h){
      quad( x, y, w, h,  0,  0, w, h);
      }

    virtual void quad( int x, int y, int  w, int  h, int tx, int ty){
      quad( x, y, w, h, tx, ty, w, h );
      }

    virtual void quad( int x, int y, int  w, int  h,
                       int tx, int ty, int tw, int th ) = 0;
    virtual void line( int x, int y, int x2, int y2) = 0;

    virtual void setBlendMode( BlendMode m );
    virtual BlendMode blendMode() const;

    virtual PaintTextEngine& textEngine() = 0;

    virtual void setNullState(){}
    virtual void pushState(){}
    virtual void popState(){}
  private:
    struct State {
      struct ScissorRect{
        int x,y,x1,y1;
        } scissor;

      Point orign;
      } rstate;
    void setState( const State & s );
    //PaintTextEngine tengine;

    friend class Painter;
    friend class Widget;
  };

class Painter {
  public:
    Painter( PaintEvent & e );
    Painter( PainterDevice & dev );
    ~Painter();

    void setScissor( int x, int y, int w, int h );
    void setScissor( const Rect &r );
    Rect scissor() const;

    void drawRect( int x, int y, int w, int h );
    void drawRect( int x, int y, int w, int h, int tx, int ty );
    void drawRect( const Rect & r );
    void drawRect( const Rect & r, int tx, int ty );

    void drawRect( int x, int y, int w, int h,
                   int tx, int ty, int tw, int th );
    void drawRect( const Rect & r,
                   const Rect & t );

    void drawRectTailed( int x, int y, int w, int h,
                         int tx, int ty, int tw, int th );

    void drawLine( int x, int y, int x1, int y1 );
    void drawLine( const Point &p, const Point &p1 );

    void translate( int dx, int dy );
    void translate( const Point &p  );

    void setTexture( const Tempest::Texture2d& );
    void setTexture( const Tempest::Sprite& );
    void unsetTexture();

    void  setColor( const Tempest::Color & cl );
    void  setColor( float r, float g, float b, float a = 1);
    Color color() const;

    void setFlip( bool h, bool v );
    bool isHorizontalFliped() const;
    bool isVerticalFliped() const;

    void setBlendMode( BlendMode );
    BlendMode blendMode() const;

    enum AlignFlag{
      NoAlign      = 0,
      AlignLeft    = 1,
      AlignHCenter = 2,
      AlignRight   = 4,

      AlignTop     = 8,
      AlignVCenter = 16,
      AlignBottom  = 32
      };

    void setFont( const std::string & f, int sz );
    void setFont( const Font &f );
    Font font() const;
    const Font::Letter& letter( const Font &f, wchar_t c );

    void drawText( int x, int y, int w, int h,
                   const char* str,
                   int flag = 0 );
    void drawText( int x, int y, const char* t, int flg = 0 );

    void drawText( int x, int y, int w, int h,
                   const std::string&,
                   int flag = 0 );
    void drawText( int x, int y, const std::string &, int flg = 0 );

    void drawText( int x, int y, int w, int h,
                   const char16_t* str,
                   int flag = 0 );
    void drawText( int x, int y, const char16_t* t, int flg = 0 );

    void drawText( int x, int y, int w, int h,
                   const std::u16string&, int flg = 0 );
    void drawText( int x, int y,
                   const std::u16string&, int flg = 0 );

    PainterDevice& device();
  private:
    PainterDevice & dev;
    PainterDevice::State oldState;

    Painter& operator = ( const Painter&){return *this;}
  };

}

#endif // PAINTER_H
