#include <Tempest/Application>
#include "mainwindow.h"

using namespace Tempest;

int main(int argc, const char** argv){ 
  Application app;
  MainWindow w;

  w.show();
  return app.exec();
  }