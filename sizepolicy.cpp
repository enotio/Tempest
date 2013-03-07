#include "sizepolicy.h"

using namespace Tempest;

SizePolicy::SizePolicy():typeH(Preferred), typeV(Preferred) {
  maxSize = maxWidgetSize();
  }

Size SizePolicy::maxWidgetSize() {
  return Size( 1<<16, 1<<16 );
  }

int Margin::xMargin() const {
  return left+right;
  }

int Margin::yMargin() const {
  return top+bottom;
  }
