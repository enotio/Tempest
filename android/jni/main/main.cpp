#include <jni.h>

#include <Tempest/Application>
#include <Tempest/Opengl2x>

#include "mainwindow.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
  return JNI_VERSION_1_6;
  }

int main(int argc, const char** argv){
  using namespace Tempest;
  Opengl2x    api;
  Application app;

  MainWindow w(api);
  w.show();

  return app.exec();
  }
