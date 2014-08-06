#include <Tempest/Application>
#include <Tempest/DirectX11>
#include "mainwindow.h"

using namespace Tempest;

int main(int argc, const char** argv){ 
  Application app;
  Tempest::DirectX11 api;
  MainWindow w(api);

  w.show();
  return app.exec();
  }