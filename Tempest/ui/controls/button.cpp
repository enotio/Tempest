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

  presAnim    = false;
  timePressed = Application::tickCount();

  setTextColor(Color(1));
  }

Button::~Button() {
  }

void Button::setIcon(const Sprite &s) {
  icn.normal   = s;
  icn.disabled = s;
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
  type = t;
  update();
  }

Button::Type Button::buttonType() const {
  return type;
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
  presAnim    = state().pressed;
  timePressed = Application::tickCount();

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
  isMouseOver = true;
  update();
  }

void Button::mouseLeaveEvent(MouseEvent &) {
  isMouseOver = false;
  update();
  }

void Button::focusEvent(FocusEvent&) {
  update();
  }

void Button::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );

  Tempest::Rect vRect = viewRect();
  Tempest::Rect r     = p.scissor();

  p.setScissor( r.intersected( vRect ) );

  const bool drawBackFrame = (buttonType()!=T_ToolButton || isMouseOver) &&
                             buttonType()!=T_FlatButton;
  if(drawBackFrame)
    drawBack(p);
  const Sprite icon = isEnabled() ? icn.normal : icn.disabled;

  if( !icon.size().isEmpty() ){
    p.setTexture( icon );

    int sz = std::min(w(), h());

    float k = std::min( sz/float(icon.w()),
                        sz/float(icon.h()) );
    k = std::min(k,1.f);

    int icW = int(icon.w()*k),
        icH = int(icon.h()*k);

    int x = std::min( ( txt.size()>0 ? margin().left:(w()-icW)/2+3), (w()-icW)/2 );

    p.drawRect( x, (h()-icH)/2, icW, icH,
                0, 0, icon.w(), icon.h() );
    }

  p.setScissor(r);

  if(drawBackFrame)
    drawFrame( p );

  p.setFont(fnt);
  p.setColor(fntColor);
  p.drawText(0, 0, w()-1, h()-1, txt,
             Tempest::AlignHCenter|Tempest::AlignVCenter );

  paintNested(e);
  finishPaint();
  }

void Button::gestureEvent(Tempest::AbstractGestureEvent &e) {
  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture& g = reinterpret_cast<Tempest::DragGesture&>(e);

    if( g.state()==Tempest::DragGesture::GestureUpdated ){
      const Point p = e.hotSpot();
      if(!(0<=p.x && 0<=p.y && p.x<w() && p.y<h()))
        setPressed(false);
      presAnim &= isPressed();
      update();
      }
    e.ignore();
    } else {
    e.ignore();
    }
  }

void Button::drawBack(Tempest::Painter &p){
  drawBack(p, viewRect());
  }

void Button::drawBack(Tempest::Painter &p, const Tempest::Rect& r ){
  auto c = p.color();
  p.setColor(Color(0.8f,0.8f,0.85f,0.75f));
  p.drawRect(Rect(r.x,r.y,r.w,r.h));
  p.setColor(c);
  }

void Button::drawFrame( Tempest::Painter &p ) {
  drawFrame(p, viewRect());
  }

void Button::drawFrame(Tempest::Painter & p, const Tempest::Rect &vRect) {
  auto c = p.color();
  p.setColor(Color(0.25,0.25,0.25,1));
  p.unsetTexture();

  p.drawLine(vRect.x,vRect.y,          vRect.x+vRect.w-1,vRect.y);
  p.drawLine(vRect.x,vRect.y+vRect.h-1,vRect.x+vRect.w-1,vRect.y+vRect.h-1);

  p.drawLine(vRect.x,          vRect.y,vRect.x,          vRect.y+vRect.h-1);
  p.drawLine(vRect.x+vRect.w-1,vRect.y,vRect.x+vRect.w-1,vRect.y+vRect.h  );
  p.setColor(c);
  }

Tempest::Rect Button::viewRect() const {
  int px = 0, py = 0,
      pw = w(), ph = h();

  if( presAnim ){
    const int s = 2;
    px += s;
    py += s;

    pw -= 2*s;
    ph -= 2*s;
    }

  return Tempest::Rect(px, py, pw, ph);
  }

void Button::keyUpEvent(Tempest::KeyEvent &e) {
  if( hasFocus() &&
      (e.key==Tempest::KeyEvent::K_Return || e.u16==32) ){
    emitClick();
    presAnim    = true;
    timePressed = Application::tickCount();
    update();
    } else {
    Widget::keyUpEvent(e);
    }
  }

void Button::onShortcut() {
  presAnim    = true;
  timePressed = Application::tickCount();
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
  return presAnim;
  }

void Button::setWidgetState(const WidgetState &s) {
  Widget::setWidgetState(s);
  update();
  }

void Button::finishPaint() {
  if( presAnim != state().pressed ){
    if( Application::tickCount() > timePressed+1000/8 )
      presAnim = state().pressed;

    update();
    }
  }
