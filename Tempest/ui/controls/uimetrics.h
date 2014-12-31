#ifndef UIMETRICS_H
#define UIMETRICS_H

namespace Tempest{
  class UiMetrics {
    public:
      UiMetrics();
      virtual ~UiMetrics() = default;

      int   buttonWidth      = 128;
      int   buttonHeight     = 27;
      float uiScale          = 1.0;

      int   smallTextSize    = 8;
      int   normalTextSize   = 16;
      int   largeTextSize    = 24;

      int   scroolButtonSize = 27;
      int   margin           = 8;
    };
  }

#endif // UIMETRICS_H
