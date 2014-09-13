#include "uimetrics.h"

using namespace Tempest;

UIMetrics::UIMetrics() {
#ifdef __MOBILE_PLATFORM__
  buttonHeight = 35;
#else
  buttonHeight = 27;
#endif
  }
