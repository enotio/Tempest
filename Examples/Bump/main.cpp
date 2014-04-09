#include <Tempest/Application>
#include <Tempest/Opengl2x>

#include "mainwindow.h"

using namespace Tempest;

int main() {
  Application app;
  Opengl2x   api;
  MainWindow w(api);

  w.show();
  return app.exec();
  }

