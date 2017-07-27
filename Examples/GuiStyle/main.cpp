#include "mainwindow.h"
#include <Tempest/Application>
#include <Tempest/Platform>

#include <Tempest/Opengl2x>

int main() {
  using namespace Tempest;
  Application app;

  Opengl2x api;

  MainWindow w( api );
  w.show();

  return app.exec();
  }

