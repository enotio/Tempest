#ifndef SIZEPOLICY_H
#define SIZEPOLICY_H

#include <Tempest/Utility>

namespace Tempest{

enum SizePolicyType{
  FixedMin,
  FixedMax,
  Preferred,
  Expanding
  };

struct SizePolicy {
  SizePolicy();

  Size minSize, maxSize;
  SizePolicyType typeH, typeV;

  static Size maxWidgetSize();
  };

struct Margin{
  Margin( int v = 0 ):left(v), right(v), top(v), bottom(v) {}
  Margin( int l, int r, int t, int b ):left(l), right(r), top(t), bottom(b) {}

  int left, right, top, bottom;

  int xMargin() const;
  int yMargin() const;

  bool operator == (const Margin& other) const {
    return left  ==other.left  &&
           right ==other.right &&
           top   ==other.top   &&
           bottom==other.bottom;
    }
  bool operator != (const Margin& other) const {
    return !(*this==other);
    }
  };
}

#endif // SIZEPOLICY_H
