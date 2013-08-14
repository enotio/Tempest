#include "application.h"

#include <Tempest/SystemAPI>
#include <Tempest/Event>

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
