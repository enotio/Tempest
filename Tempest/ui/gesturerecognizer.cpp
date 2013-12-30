#include "gesturerecognizer.h"

#include <Tempest/Event>

using namespace Tempest;

GestureRecognizer::GestureRecognizer() {
  }

GestureRecognizer::~GestureRecognizer() {
  }

AbstractGestureEvent *GestureRecognizer::event(const Event &) {
  return 0;
  }

void GestureRecognizer::deleteGesture(AbstractGestureEvent *g) {
  delete g;
  }
