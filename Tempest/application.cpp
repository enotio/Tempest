#include "application.h"

#include <Tempest/SystemAPI>
#include <Tempest/Event>

#include <Tempest/Timer>
#include <Tempest/Widget>
#include <Tempest/Assert>

#ifdef __WIN32
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <unistd.h>
#include <pthread.h>
#endif
#include <time.h>

#include <iostream>

using namespace Tempest;

Application::App Application::app;

Application::Application() {
  app.ret  = -1;
  app.quit = false;
  SystemAPI::instance().startApplication(0);
  }

Application::~Application() {
  SystemAPI::instance().endApplication();

  T_ASSERT_X( Widget::count==0, "not all widgets was destroyed");
  } 

int Application::exec() {
  execImpl(0);
  return app.ret;
  }

bool Application::isQuit() {
  return app.quit;
  }

void *Application::execImpl(void *) {
  while( !isQuit() ) {
    processEvents();
    }
  return 0;
  }

bool Application::processEvents( bool all ) {
  if( !app.quit ){
    if( all )
      app.ret = SystemAPI::instance().nextEvents(app.quit); else
      app.ret = SystemAPI::instance().nextEvent (app.quit);
    }

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
  timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  uint64_t t = (uint8_t)now.tv_sec;
  t *= 1000;
  t += now.tv_nsec/1000000;
  return t;
  }

void Application::exit() {
  app.quit = true;
  }

void Application::processTimers() {
  Application::App &a = app;

  for( size_t i=0; i<a.timer.size(); ++i ){
    uint64_t t = tickCount();

    uint64_t dt = (t-a.timer[i]->lastTimeout)/a.timer[i]->minterval;

    for( size_t r=0; r<dt && r<app.timer[i]->mrepeatCount; ++r ){
      app.timer[i]->lastTimeout = t;
      app.timer[i]->timeout();
      }
    }
  }
