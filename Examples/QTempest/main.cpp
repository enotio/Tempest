#include "mainwindow.h"
#include <QApplication>
#include <Tempest/Opengl2x>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Tempest::Opengl2x api;

  MainWindow w(api);
  w.show();

  return a.exec();
}
