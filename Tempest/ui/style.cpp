#include "style.h"

#include <Tempest/Widget>
#include <Tempest/Button>
#include <Tempest/CheckBox>
#include <Tempest/Label>
#include <Tempest/LineEdit>

using namespace Tempest;

const std::u16string Style::Extra::emptyTxt;
const Margin         Style::Extra::emptyMargin;
const Icon           Style::Extra::emptyIcon;
const Font           Style::Extra::emptyFont;
const Color          Style::Extra::emptyColor;

Style::Extra::Extra(const Widget &owner)
  : txt(emptyTxt), margin(owner.margin()), icon(emptyIcon), font(emptyFont), fontColor(emptyColor) {
  }

Style::Extra::Extra(const Button &owner)
  : txt(owner.text()), margin(owner.margin()), icon(owner.icon()), font(owner.font()), fontColor(emptyColor) {
  }

Style::Extra::Extra(const Label &owner)
  : txt(owner.text()), margin(owner.margin()), icon(emptyIcon), font(owner.font()), fontColor(owner.textColor()) {
  }

Style::Extra::Extra(const LineEdit &owner)
  : txt(owner.text()), margin(owner.margin()), icon(emptyIcon), font(owner.font()), fontColor(owner.textColor()) {
  }


Style::Style() {
  }

Style::Style(const Style *parent):parent(parent) {
  }

Style::~Style() {
  T_ASSERT(counter ==0);
  T_ASSERT(polished==0);
  setParent(nullptr);
  }

void Style::setParent(const Style *stl) {
  if( stl==this )
    stl=nullptr;

  if(stl)
    stl->addRef();
  if(parent)
    parent->decRef();
  parent=stl;
  }

void Style::polish  (Widget&) const {
  polished++;
  }

void Style::unpolish(Widget&) const {
  polished--;
  }

void Style::draw(Painter& ,Widget*, const WidgetState&, const Rect &, const Extra&) const {
  }

void Style::draw(Painter &p, Panel* w, const WidgetState &st, const Rect &r, const Extra& extra) const {
  if(parent)
    return parent->draw(p,w,st,r,extra);

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

void Style::draw(Painter& p, Button *w, const WidgetState &st, const Rect &r, const Extra &extra) const {
  if(parent)
    return parent->draw(p,w,st,r,extra);

  p.setBlendMode( Tempest::alphaBlend );
  p.translate(r.x,r.y);

  const Button::Type buttonType=st.button;

  const bool drawBackFrame = (buttonType!=Button::T_ToolButton || st.highlighted) &&
                              buttonType!=Button::T_FlatButton;
  if( drawBackFrame ) {
    auto c = p.color();
    if( st.pressed )
      p.setColor(Color(0.4f,0.4f,0.45f,0.75f)); else
      p.setColor(Color(0.8f,0.8f,0.85f,0.75f));
    p.drawRect(r);
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
    auto c = p.color();
    p.setColor(Color(0.25,0.25,0.25,1));
    p.unsetTexture();

    p.drawLine(0,0,    r.w-1,0    );
    p.drawLine(0,r.h-1,r.w-1,r.h-1);

    p.drawLine(0,      0,    0,r.h-1);
    p.drawLine(r.w-1,  0,r.w-1,r.h  );
    p.setColor(c);
    }

  p.setFont (extra.font);
  p.setColor(extra.fontColor);
  p.drawText(0, 0, r.w-1, r.h-1, extra.txt,
             Tempest::AlignHCenter|Tempest::AlignVCenter );

  p.translate(r.x,r.y);
  }

void Style::draw(Painter &p, CheckBox *w, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,st,r,extra);

  const int  s  = std::min(r.w,r.h);
  const Size sz = Size(s,s);

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

  p.setFont (extra.font);
  p.setColor(extra.fontColor);
  p.drawText(sz.w, 0, r.w-sz.w-1, r.h-1, extra.txt,
             Tempest::AlignHCenter|Tempest::AlignVCenter );
  }

void Style::draw(Painter &p, Label *w, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,st,r,extra);

  const Margin& m   = extra.margin;

  p.translate(r.x,r.y);

  p.setFont (extra.font);
  p.setColor(extra.fontColor);

  const int dY = (r.h-extra.font.size()-m.yMargin())/2;

  Rect sc = p.scissor();
  p.setScissor(sc.intersected(Rect(m.left, 0, r.w-m.xMargin(), r.h)));
  p.drawText( m.left, m.top+dY, r.w-m.xMargin(), extra.font.size(), extra.txt, AlignBottom );

  p.translate(-r.x,-r.y);
  }

void Style::draw(Painter &p, LineEdit *w, const WidgetState &st, const Rect &r, const Style::Extra &extra) const {
  if(parent)
    return parent->draw(p,w,st,r,extra);

  const char16_t passChar='*';

  const bool editable=st.editable;
  const auto emode   =st.echo;
  const bool anim    =true;
  const int  scroll  =0;

  if(emode==LineEdit::NoEcho)
    return;

  p.setFont( extra.font );

  const Margin& m = extra.margin;
  int x = m.left, y = 0;

  size_t ssel = w ? w->selectionBegin() : 0;
  size_t esel = w ? w->selectionEnd()   : 0;

  if( ssel>esel )
    std::swap(ssel,esel);

  for( size_t i=0; i<ssel && i<extra.txt.size(); ++i ){
    const Font::Letter& l = emode==LineEdit::Normal ? p.letter(extra.font,extra.txt[i]) : p.letter(extra.font,passChar);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int sx = x;

  for( size_t i=ssel;i<esel && i<extra.txt.size(); ++i ){
    const Font::Letter& l = emode==LineEdit::Normal ? p.letter(extra.font,extra.txt[i]) : p.letter(extra.font,passChar);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int oldSc = scroll;
  if( editable && sx==x ){
    --sx;
    int clientW = r.w-m.xMargin();
    int delta = 0;
    if( x+oldSc > clientW ){
      delta = clientW - (x+oldSc);
      }

    if( x+oldSc < m.left ){
      delta = -(x+oldSc-m.left);
      }

    //scroll += delta;
    sx     += delta;
    x      += delta;
    x += 1;
    }

  drawCursor(p,(ssel==esel),st,sx+oldSc,x-sx,r.h,anim);

  p.setColor(extra.fontColor);
  int dY = (r.h-extra.font.size()-m.yMargin())/2;
  Rect sc = p.scissor();
  p.setScissor(sc.intersected(Rect(m.left, 0, r.w-m.xMargin(), r.h)));
  if(emode==LineEdit::Normal){
    p.drawText( scroll+m.left, m.top+dY, r.w-m.xMargin()-scroll, extra.font.size(),
                extra.txt, AlignBottom );
    } else {
    const Font::Letter& l = p.letter(extra.font,passChar);
    int x=scroll+m.left, y = m.top+dY;
    char16_t ch[2]={passChar,'\0'};
    for(size_t i=0;i<extra.txt.size();++i){
      p.drawText(x,y,ch);
      x+= l.advance.x;
      y+= l.advance.y;
      }
    }
  }

void Style::drawCursor(Painter &p,bool emptySel, const WidgetState &st,int x1,int x2,int h,bool animState) {
  if( st.editable && ((animState || emptySel) && st.focus) ){
    p.setBlendMode(noBlend);
    p.unsetTexture();
    p.setColor( 0,0,1,1 );
    p.drawRect( x1, 0, x2, h );
    }
  }
