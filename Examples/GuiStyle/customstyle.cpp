#include "customstyle.h"

#include <Tempest/Button>
#include <Tempest/ScrollBar>

using namespace Tempest;

CustomStyle::CustomStyle(const Color &prime)
  :CustomStyle(prime,Color(0,0,0,0)){
  }

CustomStyle::CustomStyle(const Color &p, const Color &s) {
  mkColor(prime,p);
  mkColor(second,s);
  border=Color(0,0,0,0.25);
  }

void CustomStyle::polish(Widget &w) const {
  if(ScrollBar* sc=dynamic_cast<ScrollBar*>(&w))
    sc->hideArrowButtons();
  }

void CustomStyle::mkColor(Color *dest, const Color &src) {
  dest[Normal]=src;
  dest[Light ]=Color(src.r(),src.g(),src.b(),0.5);
  dest[Dark  ]=Color(src.r()/2,src.g()/2,src.b()/2);
  }

void CustomStyle::draw(Painter& p, Button*, Element /*e*/, const WidgetState &st, const Rect &r, const Extra &extra) const {
  p.setBlendMode( Tempest::alphaBlend );
  p.translate(r.x,r.y);

  const Button::Type buttonType=st.button;

  const bool drawBackFrame = (buttonType!=Button::T_ToolButton || st.highlighted) &&
                              buttonType!=Button::T_FlatButton;
  if( drawBackFrame ) {
    auto c = p.color();
    if( st.pressed )
      p.setColor(second[Dark]); else
    if( st.highlighted )
      p.setColor(second[Dark]); else
      p.setColor(second[Normal]);
    p.drawRect(0,0,r.w,r.h);
    p.setColor(c);
    }

  const int    sz   = std::min(r.w,r.h);
  const Sprite icon = extra.icon.sprite(sz,sz,st.enabled ? Icon::ST_Normal : Icon::ST_Disabled);

  if( !icon.size().isEmpty() ){
    p.setTexture( icon );

    float k = std::min( sz/float(icon.w()),
                        sz/float(icon.h()) );
    k = std::min(k,1.f);

    int icW = int(icon.w()*k),
        icH = int(icon.h()*k);

    int x = std::min( ( extra.txt.size()>0 ? extra.margin.left:(r.w-icW)/2+3), (r.w-icW)/2 );

    p.drawRect( x, (r.h-icH)/2, icW, icH,
                0, 0, icon.w(), icon.h() );
    }

  if( drawBackFrame ) {
    p.setColor(border);
    p.unsetTexture();

    p.drawLine(0,0,    r.w-1,0    );
    p.drawLine(0,r.h-1,r.w-1,r.h-1);

    p.drawLine(0,      0,    0,r.h-1);
    p.drawLine(r.w-1,  0,r.w-1,r.h  );
    }

  p.setFont (extra.font);
  p.setColor(extra.fontColor);
  p.drawText(0, 0, r.w-1, r.h-1, extra.txt,
             Tempest::AlignHCenter|Tempest::AlignVCenter );

  p.translate(r.x,r.y);
  }

void CustomStyle::draw(Painter &p, Panel*, Style::Element,
                       const WidgetState&,const Rect& r,const Style::Extra&) const {
  p.setBlendMode( Tempest::alphaBlend );
  p.setColor(Color(1.0));
  p.drawRect(r);

  p.setColor(border);
  p.unsetTexture();

  p.drawLine(0,0,    r.w-1,0    );
  p.drawLine(0,r.h-1,r.w-1,r.h-1);

  p.drawLine(0,      0,    0,r.h-1);
  p.drawLine(r.w-1,  0,r.w-1,r.h  );
  }
