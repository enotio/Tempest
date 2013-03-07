#include "event.h"

using namespace Tempest;

Event::Event() {
  accept();
  }

Point MouseEvent::pos() const {
  return Point(x,y);
  }

Size SizeEvent::size() const {
  return Size(w,h);
  }
