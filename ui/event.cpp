#include "event.h"

using namespace Tempest;

Event::Event(): accepted(true), etype(NoEvent) {
  }

Point MouseEvent::pos() const {
  return Point(x,y);
  }

Size SizeEvent::size() const {
  return Size(w,h);
  }
