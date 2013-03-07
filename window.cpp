#include "window.h"

using namespace Tempest;

Window::Window() {
  wnd = AbstractSystemAPI::instance().createWindow(640, 480);
  AbstractSystemAPI::instance().bind(wnd, this);

  pressedC = 0;
  }

Window::~Window() {
  AbstractSystemAPI::instance().deleteWindow( wnd );
  }

void Window::show() {
  AbstractSystemAPI::instance().show( wnd );
  }

void Window::setPosition(int x, int y) {
  Widget::setPosition(x,y);
  AbstractSystemAPI::instance().setGeometry( wnd, x, y, w(), h() );
  }

void Window::resize(int w, int h) {
  Widget::resize(w,h);
  AbstractSystemAPI::instance().setGeometry( wnd, x(), y(), w, h );
  }

AbstractSystemAPI::Window *Window::handle() {
  return wnd;
  }
