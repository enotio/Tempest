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


AbstractGestureEvent::AbstractGestureEvent() {
  setType(Gesture);
  setGestureType(gtNone);
  }

const Point &AbstractGestureEvent::hotSpot() const {
  return hs;
  }

void AbstractGestureEvent::setHotSpot(const Point &h) {
  hs = h;
  }

AbstractGestureEvent::GestureType AbstractGestureEvent::gestureType() const {
  return gt;
  }

void AbstractGestureEvent::setGestureType(AbstractGestureEvent::GestureType t) {
  gt = t;
  }
