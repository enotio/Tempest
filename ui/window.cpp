#include "window.h"

using namespace Tempest;

struct Window::DragGestureRecognizer : GestureRecognizer{
  DragGestureRecognizer():state(NonActivated), pointer(-1){}

  AbstractGestureEvent* event(const Event & e){
    static const double minimumDPos = 25;

    if( e.type()==Event::MouseDown &&
        state==NonActivated ){
      MouseEvent& me = (MouseEvent&)e;
      if( me.button==MouseEvent::ButtonLeft ){
        state   = Press;
        pointer = me.mouseID;
        spos    = me.pos();
        pos     = spos;
        }
      }

    if( e.type()==Event::MouseMove &&
        state==Press ){
      MouseEvent& me = (MouseEvent&)e;

      if( me.mouseID==pointer &&
          (me.pos() - spos).manhattanLength() > minimumDPos ){
        state = Move;
        pos = me.pos();
        return new DragGesture(spos, pos, Point(), AbstractGestureEvent::GestureStarted);
        }
      }

    if( e.type()==Event::MouseMove &&
        state==Move ){
      MouseEvent& me = (MouseEvent&)e;
      if( me.mouseID==pointer ){
        Point d = (me.pos()-pos);
        pos = me.pos();
        return new DragGesture(spos, pos, d, AbstractGestureEvent::GestureUpdated);
        }
      }

    if( e.type()==Event::MouseUp &&
        (state==Move || state==Press) ){
      MouseEvent& me = (MouseEvent&)e;
      if( me.mouseID==pointer ){
        state = NonActivated;
        Point d = (me.pos()-pos);
        return new DragGesture(spos, pos, d, AbstractGestureEvent::GestureFinished);
        }
      }

    return 0;
    }

  enum State{
    NonActivated,
    Press,
    Move
    };

  State state;
  int   pointer;
  Point spos, pos;
  };

Window::Window( int w, int h ) {
  init(w,h);
  wnd = SystemAPI::instance().createWindow(w, h);

  setPosition( SystemAPI::instance().windowClientPos(wnd) );
  resize( SystemAPI::instance().windowClientRect( wnd ) );

  SystemAPI::instance().bind(wnd, this);
  }

Window::Window() {
  int w = 800, h=600;
  init(w,h);
  wnd = SystemAPI::instance().createWindow(w, h);

  setPosition( SystemAPI::instance().windowClientPos(wnd) );
  resize( SystemAPI::instance().windowClientRect( wnd ) );

  SystemAPI::instance().bind(wnd, this);
  }

Window::Window( Window::ShowMode sm ) {
  init(0,0);
  smode = sm;

  if( sm==Maximized ){
    wnd = SystemAPI::instance().createWindowMaximized();
    Size s = SystemAPI::instance().windowClientRect( wnd );
    winW = s.w;
    winH = s.h;

    setPosition( SystemAPI::instance().windowClientPos(wnd) );
    resize( s.w, s.h );
    }

  if( sm==Minimized ){
    wnd = SystemAPI::instance().createWindowMinimized();
    }

  if( sm==FullScreen ){
    wnd = SystemAPI::instance().createWindowFullScr();
    Size s = SystemAPI::instance().windowClientRect( wnd );
    winW = s.w;
    winH = s.h;

    setPosition( SystemAPI::instance().windowClientPos(wnd) );
    resize( s.w, s.h );
    }

  SystemAPI::instance().bind(wnd, this);
  }

void Window::init( int w, int h ){
  resizeIntent = false;
  pressedC.reserve(8);
  pressedC.push_back(0);
  winW = w;
  winH = h;
  isAppActive = true;

  smode = Normal;

  instalGestureRecognizer( new DragGestureRecognizer() );
  }

Window::~Window() {
  SystemAPI::instance().deleteWindow( wnd );
  }

void Window::show() {
  SystemAPI::instance().show( wnd );
  }

void Window::setPosition(int ix, int iy) {
  if( x()==ix && y()==iy ){
    return;
    }

  Widget::setPosition(ix,iy);

  if( !isFullScreenMode() ){
    SystemAPI::instance().setGeometry( wnd, ix, iy, w(), h() );
    }
  }

void Window::resize(int w, int h) {
  Widget::resize(w,h);

  if( !isFullScreenMode() )
    SystemAPI::instance().setGeometry( wnd, x(), y(), w, h );
  }

bool Window::isFullScreenMode() const {
  return showMode()==FullScreen;
  }

Window::ShowMode Window::showMode() const {
  return smode;
  }

bool Window::isActive() const {
  return isAppActive;//SystemAPI::instance().isActive(wnd);
  }

SystemAPI::Window *Window::handle() {
  return wnd;
  }

AbstractGestureEvent *Window::sendEventToGestureRecognizer( const Event &e ) {
  AbstractGestureEvent *x = 0;
  for( size_t i=0; i<recognizers.size(); ++i ){
    x = recognizers[i]->event(e);
    if( x )
      return x;
    }

  return 0;
  }
