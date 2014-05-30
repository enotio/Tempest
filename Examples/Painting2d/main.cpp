#include "mainwindow.h"
#include <Tempest/Application>

#include <Tempest/DirectX9>
#include <Tempest/Opengl2x>

enum APIType{
  OpenGL,
  Direct3D
  };

Tempest::AbstractAPI& api( APIType a ){
  using namespace Tempest;

  switch( a ){
    case OpenGL:{
      static Opengl2x api;
      return api;
      }

    case Direct3D:{
      static DirectX9 api;
      return api;
      }

    default:{
      static Opengl2x api;
      return api;
      }
    }
  }

int main() {
  using namespace Tempest;
  Application app;

  MainWindow w( api(Direct3D) );
  w.show();

  return app.exec();
  }

