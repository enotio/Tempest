#include "application.h"

#include <Tempest/SystemAPI>
#include <Tempest/Event>

#include <Tempest/Timer>

#ifdef __WIN32
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <unistd.h>
#endif
#include <time.h>

using namespace Tempest;

Application::App Application::app;

Application::Application() {
  app.ret  = -1;
  app.quit = false;
  SystemAPI::instance().startApplication(0);
  }

Application::~Application() {
  SystemAPI::instance().endApplication();
  } 

int Application::exec() {
  while( !app.quit ) {
    processEvents();
    }
  return app.ret;
  }

bool Application::processEvents() {
  if( !app.quit )
    app.ret = SystemAPI::instance().nextEvent(app.quit);

  processTimers();
  return app.quit;
  }

void Application::sleep(unsigned int msec) {
#ifdef __WIN32
  Sleep(msec);
#else
  if( msec>=1000)
    sleep(msec/1000);

  if( msec%1000 )
    usleep( 1000*(msec%1000) );
#endif
  }

uint64_t Application::tickCount() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (uint8_t)now.tv_sec*1000000000LL + now.tv_nsec;
  }

void Application::processTimers() {
  uint64_t t = tickCount();

  for( size_t i=0; i<app.timer.size(); ++i ){
    uint64_t dt = t-app.timer[i]->lastTimeout;

    if( dt > app.timer[i]->interval ){
      app.timer[i]->lastTimeout = t;
      app.timer[i]->timeout();
      }
    }
  }
