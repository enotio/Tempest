#include "checkbox.h"

#include <Tempest/Application>
#include <Tempest/UIMetrics>

using namespace Tempest;

CheckBox::CheckBox() {
  const UiMetrics& m = Application::uiMetrics();

  state    = false;
  Size sz  = checkIconSize();

  setMinimumSize( int(sz.w*m.uiScale),
                  int(sz.h*m.uiScale) );
  setMaximumSize( std::max(maxSize().w, minSize().w),
                  std::max(maxSize().h, minSize().h) );
  }

void CheckBox::setChecked( bool c ) {
  if( state!=c ){
    emitClick();
    }
  }

bool CheckBox::isChecked() const {
  return state;
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

  if( state ){
    int x = 0,
        y = (h()- sz.h)/2;
    if( isPressed() ){
      p.drawLine(x+2, y+2, x+sz.w-2, y+sz.h-2);
      p.drawLine(x+sz.w-2, y+2, x+2, y+sz.h-2);
      } else {
      p.drawLine(x, y, x+sz.w, y+sz.h);
      p.drawLine(x+sz.w, y, x, y+sz.h);
      }
    }

  Button::paintEvent(e);
  }

void CheckBox::emitClick() {
  state = !state;

  Button::emitClick();
  onChecked(state);
  }

Size CheckBox::checkIconSize() const {
  return Size(20,20);
  }

void CheckBox::drawFrame(Tempest::Painter &) {}
void CheckBox::drawBack(Tempest::Painter &) {}

