#include "uimetrics.h"

#include <Tempest/Platform>
#include <Tempest/Android>

using namespace Tempest;

UiMetrics::UiMetrics() {
#ifdef __MOBILE_PLATFORM__
  buttonHeight = 48;
#else
  buttonHeight = 27;
#endif
  }

int UiMetrics::scaledSize(int x) {
#ifdef __ANDROID__
  float dpy=Tempest::AndroidAPI::densityDpi();
  return int(float(x)*dpy);
#else
  return x;
#endif
  }
