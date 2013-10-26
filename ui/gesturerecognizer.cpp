#include "gesturerecognizer.h"

using namespace Tempest;

GestureRecognizer::GestureRecognizer() {
  }

GestureRecognizer::~GestureRecognizer() {
  }

AbstractGestureEvent *GestureRecognizer::event(const Event &) {
  return 0;
  }
