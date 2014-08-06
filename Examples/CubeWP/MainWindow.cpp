#include "MainWindow.h"

using namespace Tempest;

MainWindow::MainWindow( const AbstractAPI & dx )
  :device( dx, handle() ){
  
  }

void MainWindow::render(){
  if(!device.startRender())
    return;

  device.clear( Color( 0, 0, 1 ), 1 );

  device.present();
  }