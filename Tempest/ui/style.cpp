#include "style.h"

#include <Tempest/Platform>

#include <Tempest/Widget>
#include <Tempest/Button>
#include <Tempest/CheckBox>
#include <Tempest/Label>
#include <Tempest/LineEdit>

using namespace Tempest;

const Margin         Style::Extra::emptyMargin;
const Icon           Style::Extra::emptyIcon;
const Font           Style::Extra::emptyFont;
const Color          Style::Extra::emptyColor;

Style::Extra::Extra(const Widget &owner)
  : margin(owner.margin()), icon(emptyIcon), font(emptyFont), fontColor(emptyColor) {
  }

Style::Extra::Extra(const Button &owner)
  : margin(owner.margin()), icon(owner.icon()), font(owner.font()), fontColor(emptyColor) {
  }

Style::Extra::Extra(const Label &owner)
  : margin(owner.margin()), icon(emptyIcon), font(owner.font()), fontColor(owner.textColor()) {
  }

Style::Extra::Extra(const LineEdit &owner)
  : margin(owner.margin()), icon(emptyIcon), font(owner.font()), fontColor(owner.textColor()) {
  }


Style::Style() {
  }

Style::Style(const Style *parent):parent(parent) {
  }

Style::~Style() {
  T_ASSERT(polished==0);
  }

Style::UIIntefaceIdiom Style::idiom() const {
#ifdef __MOBILE_PLATFORM__
  return UIIntefaceIdiom(UIIntefacePhone);
#else
  return UIIntefaceIdiom(UIIntefaceUnspecified);
#endif
  }

void Style::setParent(const std::shared_ptr<const Style> &stl) {
  if( stl.get()==this ) {
    parent=nullptr;
    return;
    }
  parent=stl;
  }

void Style::polish  (Widget& w) const {
  polished++;
  if(parent)
    parent->polish(w);
  }

void Style::unpolish(Widget& w) const {
  if(parent)
    parent->unpolish(w);
  polished--;
  }

const Tempest::Sprite &Style::iconSprite(const Icon& icon,const WidgetState &st, const Rect &r) {
  const int sz = std::min(r.w,r.h);
  return icon.sprite(sz,sz,st.enabled ? Icon::ST_Normal : Icon::ST_Disabled);
  }

void Style::draw(Painter& ,Widget*, Element, const WidgetState&, const Rect &, const Extra&) const {
  }

void Style::draw(Painter &p, Panel* w, Element e, const WidgetState &st, const Rect &r, const Extra& extra) const {
  if(parent)
    return parent->draw(p,w,e,st,r,extra);

  p.setBlendMode(alphaBlend);
  p.translate(r.x,r.y);

  p.setColor(Color(0.8f,0.8f,0.85f,0.75f));
  p.drawRect(0,0,r.w,r.h);

  p.setColor(Color(0.25,0.25,0.25,1));

  p.drawLine(0,0,    r.w-1,0);
  p.drawLine(0,r.h-1,r.w-1,r.h-1);

  p.drawLine(0,    0,0    ,r.h-1);
  p.drawLine(r.w-1,0,r.w-1,r.h  );

  p.translate(-r.x,-r.y);
  }

void Style::draw(Painter& p, Button *w, Element e, const WidgetState &st, const Rect &r, const Extra &extra) const {
  if(parent)
    return parent->draw(p,w,e,st,r,extra);

  p.setBlendMode( Tempest::alphaBlend );
  p.translate(r.x,r.y);

  const Button::Type buttonType=st.button;

  const bool drawBackFrame = (buttonType!=Button::T_ToolButton || st.highlighted) &&
                              buttonType!=Button::T_FlatButton;
  if( drawBackFrame ) {
    auto c = p.color();
    if( st.pressed )
      p.setColor(Color(0.4f,0.4f,0.45f,0.75f)); else
    if(st.highlighted)
      p.setColor(Color(0.5f,0.5f,0.55f,0.75f)); else
      p.setColor(Color(0.8f,0.8f,0.85f,0.75f));
    p.drawRect(0,0,r.w,r.h);
    p.setColor(c);
    }

  if( drawBackFrame ) {
    auto c = p.color();
    p.setColor(Color(0.25,0.25,0.25,1));
    p.unsetTexture();

    p.drawLine(0,0,    r.w-1,0    );
    p.drawLine(0,r.h-1,r.w-1,r.h-1);

    p.drawLine(0,      0,    0,r.h-1);
    p.drawLine(r.w-1,  0,r.w-1,r.h  );
    p.setColor(c);
    }

  if( e==E_ArrowUp || e==E_ArrowDown || e==E_ArrowLeft || e==E_ArrowRight ){
    auto c = p.color();
    p.setColor(Color(0.1f,0.1f,0.15f,1));
    p.unsetTexture();

    int cx=r.w/2;
    int cy=r.h/2;
    const int dx=(r.w-4)/2;
    const int dy=(r.h-4)/2;

    if(e==E_ArrowUp) {
      cy-=dy/2;
      p.drawLine(cx-dx,cy+dy, cx,cy);
      p.drawLine(cx,cy, cx+dx,cy+dy);
      } else
    if(e==E_ArrowDown) {
      cy+=dy/2;
      p.drawLine(cx-dx,cy-dy, cx,cy);
      p.drawLine(cx,cy, cx+dx,cy-dy);
      } else
    if(e==E_ArrowLeft) {
      cx-=dx/2;
      p.drawLine(cx+dx,cy-dy, cx,cy);
      p.drawLine(cx,cy, cx+dx,cy+dy);
      } else
    if(e==E_ArrowRight) {
      cx+=dx/2;
      p.drawLine(cx-dx,cy-dy, cx,cy);
      p.drawLine(cx,cy, cx-dx,cy+dy);
      }

    p.setColor(c);
    }

  p.translate(r.x,r.y);
  }

void Style::draw(Painter &p, CheckBox *w, Element e, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,e,st,r,extra);

  const int  s  = std::min(r.w,r.h);
  const Size sz = Size(s,s);

  p.translate(r.x,r.y);
  p.setBlendMode( Tempest::alphaBlend );
  p.setColor(Color(0.25,0.25,0.25,1));

  p.drawLine(0,     0,sz.w-1,     0);
  p.drawLine(0,sz.h-1,sz.w-1,sz.h-1);

  p.drawLine(     0,0,     0,sz.h-1);
  p.drawLine(sz.w-1,0,sz.w-1,sz.h  );

  p.setColor(Color(1));

  if( st.checked==WidgetState::Checked ) {
    int x = 0,
        y = (r.h-sz.h)/2;
    int d = st.pressed ? 2 : 4;
    if( st.pressed ) {
      p.drawLine(x+d, y+d, x+sz.w-d, y+sz.h-d);
      p.drawLine(x+sz.w-d, y+d, x+d, y+sz.h-d);
      } else {
      p.drawLine(x, y, x+sz.w, y+sz.h);
      p.drawLine(x+sz.w, y, x, y+sz.h);
      }
    } else
  if( st.checked==WidgetState::PartiallyChecked ) {
    int x = 0,
        y = (r.h-sz.h)/2;
    int d = st.pressed ? 2 : 4;
    p.drawLine(x+d, y+d,      x+sz.w-d, y+d);
    p.drawLine(x+d, y+sz.h+d, x+sz.w-d, y+sz.h+d);

    p.drawLine(x+d, y+d, x+d, y+sz.h-d);
    p.drawLine(x+sz.w+d, y+d, x+sz.w+d, y+sz.h-d);
    }
  p.translate(-r.x,-r.y);
  }

void Style::draw(Painter &p, Label *w, Element e, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,e,st,r,extra);
  }

void Style::draw(Painter &p, LineEdit *w, Element e, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,e,st,r,extra);
  // drawCursor(p,(ssel==esel),st,sx+oldSc,x-sx,r.h,anim);
  }

void Style::draw(Painter &p, ScrollBar*, Element e, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(e==E_Background)
    return;
  draw(p,static_cast<Button*>(nullptr),e,st,r,extra);
  }

void Style::draw(Painter &p, const std::u16string &text, Style::TextElement e,
                 const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,text,e,st,r,extra);

  const Margin& m = extra.margin;

  p.translate(r.x,r.y);

  const Sprite& icon = iconSprite(extra.icon,st,r);
  int dX=0;

  if( !icon.size().isEmpty() ){
    p.setTexture( icon );
    if( text.empty() ) {
      p.drawRect( (r.w-icon.w())/2, (r.h-icon.h())/2, icon.w(), icon.h() );
      } else {
      p.drawRect( m.left, (r.h-icon.h())/2, icon.w(), icon.h() );
      dX=icon.w();
      }
    }

  if(e==TE_CheckboxTitle){
    const int s = std::min(r.w,r.h);
    if( dX<s )
      dX=s;
    }

  if(dX!=0)
    dX+=8/*padding*/;

  p.setFont (extra.font);
  p.setColor(extra.fontColor);
  const int h=extra.font.textSize(text).h;
  int flag=AlignBottom;
  if(e==TE_ButtonTitle)
    flag |= AlignHCenter;

  p.drawText( m.left+dX, (r.h-h)/2, r.w-m.xMargin()-dX, h, text, flag );

  p.translate(-r.x,-r.y);
  }

void Style::draw(Painter &p, const TextModel &text, Style::TextElement e,
                 const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,text,e,st,r,extra);

  const Margin& m  = extra.margin;
  const Rect    sc = p.scissor();

  p.setScissor(sc.intersected(Rect(m.left, 0, r.w-m.xMargin(), r.h)));
  p.translate(m.left,m.right);
  text.paint(p,extra.fontColor,Color(0,0,1),st.echo);
  p.translate(-m.left,-m.right);
  p.setScissor(sc);
  }

void Style::drawCursor(Painter &p,bool emptySel, const WidgetState &st,int x1,int x2,int h,bool animState) {
  if( st.editable && ((animState || emptySel) && st.focus) ){
    p.setBlendMode(noBlend);
    p.unsetTexture();
    p.setColor( 0,0,1,1 );
    p.drawRect( x1, 0, x2, h );
    }
  }

Style::UIIntefaceIdiom::UIIntefaceIdiom(Style::UIIntefaceCategory category):category(category){
  touch=(category==UIIntefacePad || category==UIIntefacePhone);
  }
