#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Painter>

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
   :device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device),
    spHolder (texHolder),
    uiRender (shHolder) {
  texture = texHolder.load("data/texture.png");
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setTexture(texture);
  p.drawRect( Rect(100,100, 256, 256), texture.rect() );

  p.setFont( Font("data/arial", 16) );
  p.drawText(100, 80, "This is cat!");
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  uiRender.buildVbo(*this, vboHolder, iboHolder, spHolder );
  device.clear( Color(0,0,1), 1 );

  device.beginPaint();
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }
