#include "mainwindow.h"

#include <Tempest/Painter>
#include <Tempest/Application>

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI& api)
  : device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device),
    spHolder (texHolder),
    uiRender (shHolder)  {

  }

void MainWindow::paintEvent(Tempest::PaintEvent &e) {
  Painter p(e);

  p.setColor(0,0, (Application::tickCount()%1000)/1000.f );
  p.drawRect(0,0,w(),h());
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  uiRender.buildVbo(*this, vboHolder, iboHolder, spHolder );
  device.clear( Color(1,0,1), 1 );

  device.beginPaint();
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

