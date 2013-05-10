#include "window.h"

using namespace Tempest;

Window::Window( int w, int h ) {
  init(w,h);
  wnd = SystemAPI::instance().createWindow(w, h);
  resize( SystemAPI::instance().windowClientRect( wnd ) );

  SystemAPI::instance().bind(wnd, this);
  }

Window::Window() {
  int w = 800, h=600;
  init(w,h);
  wnd = SystemAPI::instance().createWindow(w, h);
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

    resize( s.w, s.h );
    }

  SystemAPI::instance().bind(wnd, this);
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
  SystemAPI::instance().deleteWindow( wnd );
  }

void Window::show() {
  SystemAPI::instance().show( wnd );
  }

void Window::setPosition(int x, int y) {
  if( !isFullScreenMode() ){
    Widget::setPosition(x,y);
    SystemAPI::instance().setGeometry( wnd, x, y, w(), h() );
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

SystemAPI::Window *Window::handle() {
  return wnd;
  }
