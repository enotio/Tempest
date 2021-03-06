#ifndef PAINTTEXTENGINE_H
#define PAINTTEXTENGINE_H

#include <string>
#include <Tempest/Font>

namespace Tempest {

enum AlignFlag{
  NoAlign      = 0,
  AlignLeft    = 1,
  AlignHCenter = 2,
  AlignRight   = 4,

  AlignTop     = 8,
  AlignVCenter = 16,
  AlignBottom  = 32
  };

namespace Bind{
  class UserFont;
  }

class PaintTextEngine {
  public:
    virtual ~PaintTextEngine(){}

    virtual void setFont( const std::string & f, int sz ){
      (void)f;
      (void)sz;
      }
    virtual void setFont( const Font & f ){ (void)f; }
    virtual Font font() const = 0;
    virtual const Font::Letter& letter( const Font &f, char16_t c ) = 0;

    virtual void drawText( int x, int y, int w, int h, const char*  ,
                           int align = NoAlign ){
      (void)x;
      (void)y;
      (void)w;
      (void)h;
      (void)align;
      }
    virtual void drawText( int x, int y, int w, int h, const char16_t* ,
                           int align = NoAlign ){
      (void)x;
      (void)y;
      (void)w;
      (void)h;
      (void)align;
      }

    virtual void setScissor( int x, int y, int w, int h ){
      (void)x;
      (void)y;
      (void)w;
      (void)h;
      }

  virtual void pushState()    = 0;
  virtual void popState()     = 0;
  virtual void setNullState() = 0;
  };

}

#endif // PAINTTEXTENGINE_H
