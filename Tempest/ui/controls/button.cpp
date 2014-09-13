#include "button.h"

using namespace Tempest;

#include <Tempest/Utility>
#include <Tempest/Application>

Button::Button()
       : hotKey(this, Tempest::KeyEvent::K_NoKey) {
  setFocusPolicy( Tempest::ClickFocus );

  UIMetrics& m = Application::uiMetrics();
  const int h = int(m.buttonHeight*m.uiScale);

  fnt = Application::mainFont();
  fnt.setSize(int(m.normalTextSize*m.uiScale));

  Tempest::SizePolicy p;
  p.maxSize = Tempest::Size(int(m.buttonWidth*m.uiScale), h);
  p.minSize = Tempest::Size(h, h);
  p.typeV   = Tempest::FixedMax;
  p.typeH   = Tempest::FixedMax;

  setSizePolicy(p);

  pressed  = false;
  presAnim = false;

  onFocusChange.bind( *this, &Button::focusChange );
  timePressed = clock();
  }

const Shortcut& Button::shortcut() const {
  return hotKey;
  }

void Button::setShortcut(const Tempest::Shortcut &sc) {
  hotKey = sc;
  hotKey.activated.bind( this, &Button::emitClick );
  hotKey.activated.bind( this, &Button::onShortcut);
  }

const std::u16string Button::text() const {
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

const std::u16string &Button::hint() const {
  return hnt;
  }

void Button::setFont(const Tempest::Font &f) {
  fnt = f;
  }

const Tempest::Font &Button::font() const {
  return fnt;
  }

void Button::mouseDownEvent(Tempest::MouseEvent &) {
  pressed  = true;
  presAnim = true;
  timePressed = clock();

  update();
  }

void Button::mouseMoveEvent( Tempest::MouseEvent & ) {
  Tempest::Point p = mapToRoot(Tempest::Point());
  Application::showHint(hnt, Tempest::Rect(p.x, p.y, w(), h()));
  }

void Button::mouseUpEvent(Tempest::MouseEvent &e) {
  if( e.x <= w() && e.y <=h() &&  e.x >=0 && e.y >=0 &&
      pressed ){
    emitClick();
    }

  pressed = false;
  update();
  }

void Button::mouseDragEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void Button::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );

  Tempest::Rect vRect = viewRect();
  Tempest::Rect r = p.scissor();

  p.setScissor( r.intersected( vRect ) );

  drawBack(p);

  if( !icon.size().isEmpty() ){
    p.setTexture( icon );

    int sz = std::min(w(), h());

    float k = std::min( sz/float(icon.w()),
                        sz/float(icon.h()) );
    k = std::min(k,1.f);

    int icW = int(icon.w()*k),
        icH = int(icon.h()*k);

    int x = std::min( ( txt.size() ? 0:(w()-icW)/2+3), (w()-icW)/2 );

    p.drawRect( x, (h()-icH)/2, icW, icH,
                0, 0, icon.w(), icon.h() );
    }

  p.setScissor(r);

  drawFrame( p );

  p.setFont(fnt);
  p.drawText( 0,0,w()-1,h()-1, txt,
              Tempest::AlignHCenter | Tempest::AlignVCenter );

  finishPaint();
  }

void Button::gestureEvent(Tempest::AbstractGestureEvent &e) {
  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture& g = (Tempest::DragGesture&)e;

    if( g.state()==Tempest::DragGesture::GestureUpdated ){
      pressed  = false;
      presAnim = false;
      update();
      }
    e.ignore();
    } else {
    e.ignore();
    }
  }

void Button::drawBack(Tempest::Painter &p){
  Tempest::Rect r = viewRect();
  static const int d = 3;
  r.x += d;
  r.y += d;

  r.w -= 2*d;
  r.h -= 2*d;

  drawBack(p, r);
  }

void Button::drawBack(Tempest::Painter &p, const Tempest::Rect& r ){
  p.setColor(Color(0.8,0.8,0.85,0.75));
  p.drawRect(r,Rect());
  }

void Button::drawFrame( Tempest::Painter &p ) {
  drawFrame(p, viewRect());
  }

void Button::drawFrame( Tempest::Painter & p, const Tempest::Rect &vRect ) {
  p.setColor(Color(0.5,0.5,0.5,1));

  p.drawLine(vRect.x,vRect.y,        vRect.x+vRect.w,vRect.y);
  p.drawLine(vRect.x,vRect.y+vRect.h,vRect.x+vRect.w,vRect.y+vRect.h);

  p.drawLine(vRect.x,        vRect.y,vRect.x,        vRect.y+vRect.h);
  p.drawLine(vRect.x+vRect.w,vRect.y,vRect.x+vRect.w,vRect.y+vRect.h);
  }

Tempest::Rect Button::viewRect() const {
  int px = 0, py = 0,
      pw = w(), ph = h();

  if( presAnim ){
    const int s = 1;
    px += s;
    py += s;

    pw -= 2*s;
    ph -= 2*s;
    }

  return Tempest::Rect(px, py, pw, ph);
  }

void Button::keyPressEvent(Tempest::KeyEvent &e) {
  if( hasFocus() && e.key==Tempest::KeyEvent::K_Return ){
    emitClick();
    presAnim = true;
    update();
    } else
    e.ignore();
  }

void Button::focusChange( bool ) {
  update();
  }

void Button::onShortcut() {
  presAnim = true;
  timePressed = clock();
  update();
  }

void Button::emitClick() {
  clicked();
  }

bool Button::isPressed() const {
  return presAnim;
  }

void Button::finishPaint() {
  if( presAnim != pressed ){
    if( clock() > timePressed+CLOCKS_PER_SEC/8 )
      presAnim = pressed;

    update();
    }
  }
