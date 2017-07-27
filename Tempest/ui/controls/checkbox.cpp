#include "checkbox.h"

#include <Tempest/Application>
#include <Tempest/UIMetrics>

using namespace Tempest;

CheckBox::CheckBox() {
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

void CheckBox::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  style().draw(p,this,  Style::E_Background,    Button::state(),Rect(0,0,w(),h()),Style::Extra(*this));
  style().draw(p,text(),Style::TE_CheckboxTitle,Button::state(),Rect(0,0,w(),h()),Style::Extra(*this));

  paintNested(e);
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
