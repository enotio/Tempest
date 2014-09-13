#ifndef UIMETRICS_H
#define UIMETRICS_H

namespace Tempest{
  class UIMetrics {
    public:
      UIMetrics();
      virtual ~UIMetrics() = default;

      int   buttonWidth    = 128;
      int   buttonHeight   = 27;
      float uiScale        = 1.0;

      int   smallTextSize  = 8;
      int   normalTextSize = 16;
      int   largeTextSize  = 24;
    };
  }

#endif // UIMETRICS_H
