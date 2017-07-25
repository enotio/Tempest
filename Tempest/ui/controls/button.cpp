#include "button.h"

using namespace Tempest;

#include <Tempest/Utility>
#include <Tempest/Application>
#include <Tempest/Menu>

Button::Button()
       : hotKey(this, Tempest::KeyEvent::K_NoKey) {
  setFocusPolicy( Tempest::TabFocus );

  const UiMetrics& m = Application::uiMetrics();
  const int  h = int(m.buttonHeight*m.uiScale);

  fnt = Application::mainFont();
  fnt.setSize(int(m.normalTextSize*m.uiScale));

  Tempest::SizePolicy p;
  p.maxSize = Tempest::Size(int(m.buttonWidth*m.uiScale), h);
  p.minSize = Tempest::Size(h, h);
  p.typeV   = Tempest::FixedMax;
  p.typeH   = Tempest::FixedMax;
  resize(int(m.buttonWidth*m.uiScale), h);

  setMargin(int(m.margin*m.uiScale));

  setSizePolicy(p);
  setTextColor(Color(1));
  }

Button::~Button() {
  }

void Button::setIcon(const Sprite &s) {
  icn=Icon();
  icn.set(Icon::ST_Normal,  s);
  icn.set(Icon::ST_Disabled,s);
  }

void Button::setIcon(const Icon &s) {
  icn = s;
  }

const Icon &Button::icon() const {
  return icn;
  }

const Shortcut& Button::shortcut() const {
  return hotKey;
  }

void Button::setShortcut(const Tempest::Shortcut &sc) {
  hotKey = sc;
  hotKey.activated.bind(this, &Button::emitClick );
  hotKey.activated.bind(this, &Button::onShortcut);
  }

const std::u16string &Button::text() const {
  return txt;
  }

void Button::setText(const std::u16string &t) {
  if( txt!=t ){
    txt = t;
    update();
    }
  }

void Button::setText(const std::string &t) {
  setText( Tempest::SystemAPI::toUtf16( t ) );
  }

void Button::setHint(const std::u16string &str) {
  hnt = str;
  }

void Button::setHint(const std::string &str) {
  setHint( Tempest::SystemAPI::toUtf16( str ) );
  }

const std::u16string &Button::hint() const {
  return hnt;
  }

void Button::setFont(const Tempest::Font &f) {
  fnt = f;
  }

const Tempest::Font& Button::font() const {
  return fnt;
  }

void Button::setTextColor(const Color &color) {
  fntColor = color;
  update();
  }

const Color& Button::textColor() const {
  return fntColor;
  }

void Button::setButtonType(Button::Type t) {
  auto st = state();
  st.button = t;
  setWidgetState(st);
  }

Button::Type Button::buttonType() const {
  return state().button;
  }

void Button::setMenu(Menu* m) {
  btnMenu.reset(m);
  }

Menu* Button::menu() const {
  return btnMenu.get();
  }

void Button::showMenu() {
  if( btnMenu )
    btnMenu->exec(*this);
  }

void Button::mouseDownEvent(Tempest::MouseEvent& e) {
  if(!isEnabled())
    return;
  setPressed(e.button==Event::ButtonLeft);
  update();
  }

void Button::mouseMoveEvent( Tempest::MouseEvent & ) {
  Tempest::Point p = mapToRoot(Tempest::Point());
  Application::showHint(hnt, Tempest::Rect(p.x, p.y, w(), h()));
  }

void Button::mouseUpEvent(Tempest::MouseEvent &e) {
  if( e.x<=w() && e.y<=h() &&  e.x>=0 && e.y>=0 && isEnabled() ){
    if(isPressed())
      emitClick();
    else if(e.button==Event::ButtonRight)
      showMenu();
    }

  setPressed(false);
  update();
  }

void Button::mouseDragEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void Button::mouseEnterEvent(MouseEvent &) {
  auto st = state();
  st.highlighted = true;
  setWidgetState(st);

  update();
  }

void Button::mouseLeaveEvent(MouseEvent &) {
  auto st = state();
  st.highlighted = false;
  setWidgetState(st);
  }

void Button::focusEvent(FocusEvent& e) {
  Widget::focusEvent(e);
  update();
  }

void Button::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  style().draw(p,this,state(),Rect(0,0,w(),h()),Style::Extra(*this));

  paintNested(e);
  }

void Button::gestureEvent(Tempest::AbstractGestureEvent &e) {
  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture& g = reinterpret_cast<Tempest::DragGesture&>(e);

    if( g.state()==Tempest::DragGesture::GestureUpdated ){
      const Point p = e.hotSpot();
      if(!(0<=p.x && 0<=p.y && p.x<w() && p.y<h()))
        setPressed(false);
      //presAnim &= isPressed();
      update();
      }
    e.ignore();
    } else {
    e.ignore();
    }
  }

void Button::keyUpEvent(Tempest::KeyEvent &e) {
  if( hasFocus() &&
      (e.key==Tempest::KeyEvent::K_Return || e.u16==32) ){
    emitClick();
    update();
    } else {
    Widget::keyUpEvent(e);
    }
  }

void Button::onShortcut() {
  update();
  }

void Button::emitClick() {
  if(isEnabled())
    onClicked();
  }

void Button::setPressed(bool p) {
  if(state().pressed==p)
    return;
  auto st=state();
  st.pressed=p;
  setWidgetState(st);
  }

bool Button::isPressed() const {
  return state().pressed;
  }

void Button::setWidgetState(const WidgetState &s) {
  Widget::setWidgetState(s);
  update();
  }
