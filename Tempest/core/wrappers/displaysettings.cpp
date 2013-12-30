#include "displaysettings.h"

#include <Tempest/Utility>

using namespace Tempest;

DisplaySettings::DisplaySettings( int w, int h, int bits, bool f )
  : width(w), height(h), bits(bits), fullScreen(f) {
  }

DisplaySettings::DisplaySettings(const Size &s, int bits, bool f)
  : width(s.w), height(s.h), bits(bits), fullScreen(f) {
  }

Size DisplaySettings::size() const {
  return Size(width,height);
  }

void DisplaySettings::setSize(const Size &s) {
  width  = s.w;
  height = s.h;
  }
