#ifndef WIDGETSTATE_H
#define WIDGETSTATE_H

#include <cstdint>

namespace Tempest {
  class WidgetState {
    public:
      enum CheckState : uint8_t {
        Unchecked        = 0,
        Checked          = 1,
        PartiallyChecked = 2
        };

      WidgetState();

      // all widgets
      bool enabled = true;
      bool focus   = false;
      bool visible = true;

      // click-controls
      bool       pressed = false;
      CheckState checked = Unchecked;
    };
  }

#endif // WIDGETSTATE_H
