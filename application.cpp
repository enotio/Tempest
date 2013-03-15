#include "application.h"

#include <Tempest/AbstractSystemAPI>
#include <Tempest/Event>

using namespace Tempest;

Application::Application() {
  AbstractSystemAPI::instance().startApplication(0);
  }

Application::~Application() {
  AbstractSystemAPI::instance().endApplication();
  } 

int Application::exec() {
  bool quit = 0;
  int ret;

  while( !quit ) {
    ret = AbstractSystemAPI::instance().nextEvent(quit);
    }

  return ret;
  }
