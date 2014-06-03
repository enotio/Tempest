#include <Tempest/Application>
#include <Tempest/Opengl4x>

#include "mainwindow.h"

using namespace Tempest;

int main() {
  Application app;
  Opengl4x   api;
  MainWindow w(api);

  w.show();
  return app.exec();
  }

