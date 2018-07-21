#include "uimetrics.h"

#include <Tempest/Platform>
#include <Tempest/Android>
#include <Tempest/IOS>

using namespace Tempest;

UiMetrics::UiMetrics() {
#ifdef __MOBILE_PLATFORM__
  buttonHeight = 48;
#else
  buttonHeight = 27;
#endif
  }

int UiMetrics::scaledSize(int x) {
#ifdef __IOS__
  float dpy=Tempest::iOSAPI::densityDpi();
  return int(float(x)*dpy);
#elif  defined(__ANDROID__)
  float dpy=Tempest::AndroidAPI::densityDpi();
  return int(float(x)*dpy);
#else
  return x;
#endif
  }
 
