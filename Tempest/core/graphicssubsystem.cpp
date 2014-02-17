#include "graphicssubsystem.h"

using namespace Tempest;

GraphicsSubsystem::Event::Event():type(NoEvent) {
  }


GraphicsSubsystem::GraphicsSubsystem() {

  }

GraphicsSubsystem::~GraphicsSubsystem() {

  }

void GraphicsSubsystem::event(const Event &e ) {
  switch ( e.type ) {
    case Event::NoEvent:
    case Event::Count:
      break;
      
    case Event::DeleteObject:
      event( (const DeleteEvent&)e );
      break;      
    }
  }

void GraphicsSubsystem::event(const DeleteEvent &) {
  
  }
