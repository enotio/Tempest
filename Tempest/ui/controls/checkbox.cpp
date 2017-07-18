#include "checkbox.h"

#include <Tempest/Application>
#include <Tempest/UIMetrics>

using namespace Tempest;

CheckBox::CheckBox() {
  const UiMetrics& m = Application::uiMetrics();

  tristate = false;
  Size sz  = checkIconSize();

  setMinimumSize( int(sz.w*m.uiScale),
                  int(sz.h*m.uiScale) );
  setMaximumSize( std::max(maxSize().w, minSize().w),
                  std::max(maxSize().h, minSize().h) );

  setFocusPolicy(StrongFocus);
  }

void CheckBox::setChecked( bool c ) {
  setState(c ? WidgetState::Checked : WidgetState::Unchecked);
  }

bool CheckBox::isChecked() const {
  return Button::state().checked==WidgetState::Checked;
  }

CheckBox::State CheckBox::state() const {
  return Button::state().checked;
  }

void CheckBox::setState(CheckBox::State s) {
  if(s==WidgetState::PartiallyChecked && !tristate)
    s = WidgetState::Unchecked;
  if(s==state())
    return;

  auto st=Button::state();
  st.checked = s;
  setWidgetState(st);
  }

void CheckBox::setWidgetState(const WidgetState &nstate) {
  const State old = Button::state().checked;
  const bool  ck  = (old==WidgetState::Checked);
  WidgetState st  = nstate;

  if(st.checked==WidgetState::PartiallyChecked && !tristate)
    st.checked = WidgetState::Unchecked;

  Button::setWidgetState(st);
  const State next=state();

  if(old!=next)
    onStateChanged(next);

  if(ck!=(next==WidgetState::Checked))
    onChecked(next==WidgetState::Checked);

  update();
  }

void CheckBox::setTristate(bool y) {
  tristate = y;
  if(!tristate && Button::state().checked==WidgetState::PartiallyChecked)
    setState(Button::state().checked);
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

  if( Button::state().checked==WidgetState::Checked ){
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
  if( Button::state().checked==WidgetState::PartiallyChecked ){
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
  State st=Button::state().checked;
  const State old = st;
  const bool  ck  = (old==WidgetState::Checked);

  if(tristate)
    st = State((st+1)%3); else
    st = State((st+1)%2);
  setState(st);

  if( (old!=state() && tristate) || (ck!=(state()==WidgetState::Checked)))
    Button::emitClick();
  }

Size CheckBox::checkIconSize() const {
  return Size(20,20);
  }

void CheckBox::drawFrame(Tempest::Painter &) {}
void CheckBox::drawBack(Tempest::Painter &) {}

