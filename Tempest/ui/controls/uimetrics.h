#ifndef UIMETRICS_H
#define UIMETRICS_H

namespace Tempest {
  /** \addtogroup GUI
   *  @{
   */
  class UiMetrics {
    public:
      UiMetrics();
      virtual ~UiMetrics() = default;

      static int scaledSize(int x);

      int   buttonWidth      = 128;
      int   buttonHeight     = 27;
      float uiScale          = 1.0;

      int   smallTextSize    = 8;
      int   normalTextSize   = 16;
      int   largeTextSize    = 24;

      int   scrollButtonSize = 27;
      int   margin           = 8;
    };
  /** @}*/
  }

#endif // UIMETRICS_H
