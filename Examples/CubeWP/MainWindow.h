#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

class MainWindow:public Tempest::Window {
  public:
    MainWindow( const Tempest::AbstractAPI & dx );

  private:
    Tempest::Device device;
    void render();
  };

#endif