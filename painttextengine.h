#ifndef PAINTTEXTENGINE_H
#define PAINTTEXTENGINE_H

#include <string>

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
    virtual void setFont( const Bind::UserFont & f ){ (void)f; }

    virtual void drawText( int x, int y, int w, int h, const std::string  &,
                           int align = NoAlign ){
      (void)x;
      (void)y;
      (void)w;
      (void)h;
      (void)align;
      }
    virtual void drawText( int x, int y, int w, int h, const std::wstring &,
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
  };

}

#endif // PAINTTEXTENGINE_H
