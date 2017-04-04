#include "checkbox.h"

#include <Tempest/Application>
#include <Tempest/UIMetrics>

using namespace Tempest;

CheckBox::CheckBox() {
  const UiMetrics& m = Application::uiMetrics();

  tristate = false;
  st       = Unchecked;
  Size sz  = checkIconSize();

  setMinimumSize( int(sz.w*m.uiScale),
                  int(sz.h*m.uiScale) );
  setMaximumSize( std::max(maxSize().w, minSize().w),
                  std::max(maxSize().h, minSize().h) );
  }

void CheckBox::setChecked( bool c ) {
  setState(c ? Checked : Unchecked);
  }

bool CheckBox::isChecked() const {
  return st==Checked;
  }

CheckBox::State CheckBox::state() const {
  return st;
  }

void CheckBox::setState(CheckBox::State s) {
  bool ck = (st==Checked);
  if(s==PartiallyChecked && !tristate)
    s = Unchecked;
  if(s==st)
    return;

  st = s;
  onStateChanged(s);
  if(ck!=(st==Checked))
    onChecked(st==Checked);
  Button::emitClick();
  update();
  }

void CheckBox::setTristate(bool y) {
  tristate = y;
  if(!tristate && st==PartiallyChecked)
    setState(st);
  }

bool CheckBox::isTristate() const {
  return tristate;
  }

Tempest::Rect CheckBox::viewRect() const {
  Tempest::Rect r    = Button::viewRect();
  const UiMetrics& m = Application::uiMetrics();

  int dw = std::min( std::min(int(checkIconSize().w*m.uiScale),h()), r.w );
  r.w -= dw;
  r.x += dw;

  return r;
  }

void CheckBox::paintEvent( Tempest::PaintEvent &e ) {
  const UiMetrics& m = Application::uiMetrics();
  Size sz = checkIconSize();
  sz.w = int(sz.w*m.uiScale);
  sz.h = int(sz.h*m.uiScale);

  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );

  int y = int((h()-sz.h)/2);

  static const int ds = 3;
  Button::drawBack( p, Tempest::Rect{ds,y+ds, sz.w-ds*2, sz.w-ds*2} );
  Button::drawFrame(p, Tempest::Rect{0,y, sz.w, sz.h} );

  if( st==Checked ){
    int x = 0,
        y = (h()- sz.h)/2;
    if( isPressed() ){
      p.drawLine(x+2, y+2, x+sz.w-2, y+sz.h-2);
      p.drawLine(x+sz.w-2, y+2, x+2, y+sz.h-2);
      } else {
      p.drawLine(x, y, x+sz.w, y+sz.h);
      p.drawLine(x+sz.w, y, x, y+sz.h);
      }
    } else
  if( st==PartiallyChecked ){
    int x = 0,
        y = (h()- sz.h)/2;
    int d = isPressed() ? 2 : 4;
    p.drawLine(x+d, y+d,      x+sz.w-d, y+d);
    p.drawLine(x+d, y+sz.h+d, x+sz.w-d, y+sz.h+d);

    p.drawLine(x+d, y+d, x+d, y+sz.h-d);
    p.drawLine(x+sz.w+d, y+d, x+sz.w+d, y+sz.h-d);
    }


  Button::paintEvent(e);
  }

void CheckBox::emitClick() {
  if(!isEnabled())
    return;
  State state;
  if(tristate)
    state = State((st+1)%3); else
    state = State((st+1)%2);
  setState(state);
  }

Size CheckBox::checkIconSize() const {
  return Size(20,20);
  }

void CheckBox::drawFrame(Tempest::Painter &) {}
void CheckBox::drawBack(Tempest::Painter &) {}

