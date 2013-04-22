#include "application.h"

#include <Tempest/SystemAPI>
#include <Tempest/Event>

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
  if( app.quit )
    app.ret = SystemAPI::instance().nextEvent(app.quit);

  return app.quit;
  }
