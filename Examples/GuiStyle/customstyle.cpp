#include "customstyle.h"

#include <Tempest/Button>
#include <Tempest/ScrollBar>
#include <Tempest/TextModel>

using namespace Tempest;

CustomStyle::CustomStyle(const Color &prime)
  :CustomStyle(prime,Color(0,0,0,0)){
  }

CustomStyle::CustomStyle(const Color &p, const Color &s) {
  mkColor(prime,p);
  mkColor(second,s);
  border=Color(0,0,0,0.25);

  font = Font("data/roboto",14);
  font.setBold(true);
  }

void CustomStyle::polish(Widget &w) const {
  Style::polish(w);

  if(Button* btn=dynamic_cast<Button*>(&w)) {
    btn->setMinimumSize(48,48);
    btn->setMaximumSize(btn->maxSize().w,48);
    }

  if(ScrollBar* sc=dynamic_cast<ScrollBar*>(&w)) {
    sc->hideArrowButtons();
    sc->setLinearSize(8);
    }
  }

void CustomStyle::mkColor(Color *dest, const Color &src) {
  dest[Normal]=src;
  dest[Light ]=Color(src.r(),src.g(),src.b(),0.5);
  dest[Dark  ]=Color(src.r()/2,src.g()/2,src.b()/2);
  }

void CustomStyle::draw(Painter& p, Button*, Element /*e*/, const WidgetState &st, const Rect &r, const Extra&) const {
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

  if( drawBackFrame ) {
    p.setColor(border);
    p.unsetTexture();

    p.drawLine(0,0,    r.w-1,0    );
    p.drawLine(0,r.h-1,r.w-1,r.h-1);

    p.drawLine(0,      0,    0,r.h-1);
    p.drawLine(r.w-1,  0,r.w-1,r.h  );
    }

  p.translate(r.x,r.y);
  }

void CustomStyle::draw(Painter &p,CheckBox*,Style::Element, const WidgetState &st, const Rect &r, const Style::Extra &) const {
  const int  s  = std::min(std::min(r.w,r.h),24);
  const Size sz = Size(s,s);

  p.translate(r.x,r.y);
  p.setBlendMode( Tempest::alphaBlend );
  p.setColor(prime[Dark]);

  int x = 0,
      y = (r.h-sz.h)/2;
  p.drawLine(x,       y,x+sz.w-1,       y);
  p.drawLine(x,y+sz.h-1,x+sz.w-1,y+sz.h-1);

  p.drawLine(      x, y,       x,y+sz.h-1);
  p.drawLine(x+sz.w-1,y,x+sz.w-1,y+sz.h  );

  if( st.checked==WidgetState::Checked ) {
    int d = st.pressed ? 2 : 4;
    if( st.pressed ) {
      p.drawLine(x+d,      y+d, x+sz.w-d, y+sz.h-d);
      p.drawLine(x+sz.w-d, y+d, x+d,      y+sz.h-d);
      } else {
      p.drawLine(x, y, x+sz.w, y+sz.h);
      p.drawLine(x+sz.w, y, x, y+sz.h);
      }
    } else
  if( st.checked==WidgetState::PartiallyChecked ) {
    int d = st.pressed ? 2 : 4;
    p.drawLine(x+d, y+d,      x+sz.w-d, y+d);
    p.drawLine(x+d, y+sz.h+d, x+sz.w-d, y+sz.h+d);

    p.drawLine(x+d, y+d, x+d, y+sz.h-d);
    p.drawLine(x+sz.w+d, y+d, x+sz.w+d, y+sz.h-d);
    }
  p.translate(-r.x,-r.y);
  }

void CustomStyle::draw(Painter &p, Panel*, Style::Element,
                       const WidgetState&,const Rect& r,const Style::Extra&) const {
  p.translate(r.x,r.y);
  p.setBlendMode( Tempest::alphaBlend );
  p.setColor(Color(1.0));
  p.drawRect(0,0,r.w,r.h);

  p.setColor(border);
  p.unsetTexture();

  p.drawLine(0,0,    r.w-1,0    );
  p.drawLine(0,r.h-1,r.w-1,r.h-1);

  p.drawLine(0,      0,    0,r.h-1);
  p.drawLine(r.w-1,  0,r.w-1,r.h  );
  p.translate(-r.x,-r.y);
  }

void CustomStyle::draw(Painter &p, const std::u16string &text, Style::TextElement elt,
                       const WidgetState& st, const Rect &r, const Style::Extra &extra) const {
  const Margin& m = extra.margin;


  p.setFont (font);
  p.translate(r.x,r.y);

  const Sprite& icon = iconSprite(extra.icon,st,r);
  int dX=0;

  if( !icon.size().isEmpty() && elt!=TE_CheckboxTitle ){
    p.setColor(Color(1.f));
    p.setTexture( icon );
    if( text.empty() ) {
      p.drawRect( (r.w-icon.w())/2, (r.h-icon.h())/2, icon.w(), icon.h() );
      } else {
      p.drawRect( m.left, (r.h-icon.h())/2, icon.w(), icon.h() );
      }
    }

  switch( elt ){
    case TE_LabelTitle:
    case TE_CheckboxTitle:
      p.setColor(Color(0,0,0,1));
      break;
    default:
      p.setColor(extra.fontColor);
    }

  const Size txtSz=extra.font.textSize(text);
  p.drawText( m.left+dX+(r.w-dX-m.xMargin()-txtSz.w)/2, (r.h-txtSz.h)/2, r.w-m.xMargin(), txtSz.h, text, AlignBottom );

  p.translate(-r.x,-r.y);
  }

void CustomStyle::draw(Painter &p, const TextModel &text, Style::TextElement elt,
                       const WidgetState &st, const Rect &r, const Style::Extra &extra) const {

  const Margin& m  = extra.margin;
  const Rect    sc = p.scissor();
  Color         cl = extra.fontColor;

  if( elt==TE_LineEditContent )
    cl = Color(0,0,0,1);

  p.setScissor(sc.intersected(Rect(m.left, 0, r.w-m.xMargin(), r.h)));
  p.translate(m.left,m.right);
  text.paint(p,cl,second[Normal],st.echo);
  p.translate(-m.left,-m.right);
  p.setScissor(sc);
  }
