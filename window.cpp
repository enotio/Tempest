#include "window.h"

using namespace Tempest;

Window::Window( int w, int h ) {
  init(w,h);
  wnd = AbstractSystemAPI::instance().createWindow(w, h);
  resize( AbstractSystemAPI::instance().windowClientRect( wnd ) );

  AbstractSystemAPI::instance().bind(wnd, this);
  }

Window::Window() {
  int w = 800, h=600;
  init(w,h);
  wnd = AbstractSystemAPI::instance().createWindow(w, h);
  resize( AbstractSystemAPI::instance().windowClientRect( wnd ) );

  AbstractSystemAPI::instance().bind(wnd, this);
  }

Window::Window( Window::ShowMode sm ) {
  init(0,0);
  smode = sm;

  if( sm==Maximized ){
    wnd = AbstractSystemAPI::instance().createWindowMaximized();
    Size s = AbstractSystemAPI::instance().windowClientRect( wnd );
    winW = s.w;
    winH = s.h;

    resize( s.w, s.h );
    }

  if( sm==Minimized ){
    wnd = AbstractSystemAPI::instance().createWindowMinimized();
    }

  if( sm==FullScreen ){
    wnd = AbstractSystemAPI::instance().createWindowFullScr();
    Size s = AbstractSystemAPI::instance().windowClientRect( wnd );
    winW = s.w;
    winH = s.h;

    resize( s.w, s.h );
    }

  AbstractSystemAPI::instance().bind(wnd, this);
  }

void Window::init( int w, int h ){
  resizeIntent = false;
  pressedC = 0;
  winW = w;
  winH = h;
  isAppActive = true;

  smode = Normal;
  }

Window::~Window() {
  AbstractSystemAPI::instance().deleteWindow( wnd );
  }

void Window::show() {
  AbstractSystemAPI::instance().show( wnd );
  }

void Window::setPosition(int x, int y) {
  if( !isFullScreenMode() ){
    Widget::setPosition(x,y);
    AbstractSystemAPI::instance().setGeometry( wnd, x, y, w(), h() );
    }
  }

void Window::resize(int w, int h) {
  Widget::resize(w,h);

  if( !isFullScreenMode() )
    AbstractSystemAPI::instance().setGeometry( wnd, x(), y(), w, h );
  }

bool Window::isFullScreenMode() const {
  return showMode()==FullScreen;
  }

Window::ShowMode Window::showMode() const {
  return smode;
  }

AbstractSystemAPI::Window *Window::handle() {
  return wnd;
  }
