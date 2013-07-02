#include "model.h"

#include <cmath>

using namespace Tempest;

float ModelBounds::diameter() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-min[i])*(max[i]-min[i]);

  return sqrt(s);
  }

float ModelBounds::radius() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-mid[i])*(max[i]-mid[i]);

  return sqrt(s);
  }
