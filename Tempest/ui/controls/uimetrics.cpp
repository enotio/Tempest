#include "uimetrics.h"

#include <Tempest/Platform>

using namespace Tempest;

UiMetrics::UiMetrics() {
#ifdef __MOBILE_PLATFORM__
  buttonHeight = 48;
#else
  buttonHeight = 27;
#endif
  }
