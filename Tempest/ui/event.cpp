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


AbstractGestureEvent::AbstractGestureEvent(GestureRecognizer *owner): ow(owner) {
  setType(Gesture);
  setGestureType(gtNone);
  setState(GestureUpdated);
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

AbstractGestureEvent::State AbstractGestureEvent::state() const {
  return st;
  }

const GestureRecognizer &AbstractGestureEvent::owner() const {
  return *ow;
  }

GestureRecognizer &AbstractGestureEvent::owner() {
  return *ow;
  }

void AbstractGestureEvent::setState(AbstractGestureEvent::State s) {
  st = s;
  }
