#include "mainwindow.h"
#include <Tempest/Application>
#include <Tempest/Opengl4x>

int main() {
  using namespace Tempest;
  Application app;

  Opengl4x api;
  MainWindow w( api );
  w.show();

  return app.exec();
  }

