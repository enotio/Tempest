#include "mainwindow.h"
#include <Tempest/Application>
//#include <Tempest/Opengl2x>
#include <Tempest/DirectX11>

int main() {
  using namespace Tempest;
  Application app;

  DirectX11 api;
  MainWindow w( api );
  w.show();

  return app.exec();
  }

