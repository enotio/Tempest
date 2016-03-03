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
  image.load("data/image.svg");
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  //p.drawRect( Rect(100,100, 256, 256), texture.rect() );

  p.setFont( Font("data/arial", 16) );
  p.drawText(100, 80, "This is cat!");

  Rect s = scissor;
  if(s.w<0){
    s.x += s.w;
    s.w = -s.w;
    }
  if(s.h<0){
    s.y += s.h;
    s.h = -s.h;
    }
  p.drawRect(s);
  /*
  p.setTexture(texture);

  p.setScissor(s);
  p.drawTriangle( 100, 100,
                  428, 120,
                  138, 455);

  p.drawTriangle( 500, 500,
                  600, 500,
                  500, 600);*/
  image.paint(p);
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  uiRender.buildVbo(*this, vboHolder, iboHolder, spHolder );
  device.clear( Color(1,1,1), 1 );

  device.beginPaint();
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::mouseDownEvent(MouseEvent& e) {
  scissor.x = e.x;
  scissor.y = e.y;
  }

void MainWindow::mouseDragEvent(MouseEvent& e) {
  scissor.w = e.x - scissor.x;
  scissor.h = e.y - scissor.y;
  }
