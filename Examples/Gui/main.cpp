#include "mainwindow.h"
#include <Tempest/Application>
//#include <Tempest/Opengl2x>

#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <Tempest/DirectX11>
#else
#include <Tempest/Opengl2x>
#endif

int main() {
  using namespace Tempest;
  Application app;

#ifdef __WINDOWS__
  DirectX11 api;
#else
  Opengl2x api;
#endif

  MainWindow w( api );
  w.show();

  return app.exec();
  }

