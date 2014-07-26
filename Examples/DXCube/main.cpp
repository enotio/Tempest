#include "mainwindow.h"
#include <Tempest/Application>
#include <Tempest/DirectX9>
//#include <Tempest/DirectX11>

int main() {
  using namespace Tempest;
  Application app;

  DirectX9 api;
  MainWindow w( api );
  w.show();

  return app.exec();
  }

