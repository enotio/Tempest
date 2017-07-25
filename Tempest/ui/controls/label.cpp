#include "label.h"

#include <Tempest/Application>

using namespace Tempest;

Label::Label() {
  resize(100,27);

  const UiMetrics& uiMetrics = Application::uiMetrics();
  fnt = Application::mainFont();
  fnt.setSize( int(uiMetrics.normalTextSize*uiMetrics.uiScale) );

  SizePolicy p = sizePolicy();
  p.maxSize.h = fnt.size() + fnt.size()/2;
  p.minSize.h = p.maxSize.h;
  p.typeV = FixedMax;

  setSizePolicy(p);
  }

void Label::setFont(const Font &f) {
  fnt = f;
  }

const Font &Label::font() const {
  return fnt;
  }

void Label::setTextColor(const Color& c) {
  tColor = c;
  update();
  }

const Color& Label::textColor() const {
  return tColor;
  }

void Label::setText( const std::string &t ) {
  std::u16string s;
  s.assign( t.begin(), t.end() );
  setText( s );
  }

void Label::setText(const std::u16string &t) {
  txt = t;
  }

void Label::paintEvent(PaintEvent &e) {
  Painter p(e);
  style().draw(p,this,state(),Rect(0,0,w(),h()),Style::Extra(*this));

  paintNested(e);
  }

const std::u16string &Label::text() const {
  return txt;
  }

