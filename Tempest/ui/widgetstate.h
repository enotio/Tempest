#ifndef WIDGETSTATE_H
#define WIDGETSTATE_H

#include <cstdint>

namespace Tempest {
  class WidgetState {
    public:
      enum ButtonType : uint8_t {
        T_PushButton = 0,
        T_ToolButton = 1,
        T_FlatButton = 2
        };

      enum CheckState : uint8_t {
        Unchecked        = 0,
        Checked          = 1,
        PartiallyChecked = 2
        };

      enum EchoMode : uint8_t {
        Normal   = 0,
        NoEcho   = 1,
        Password = 2
        };

      WidgetState();

      // all widgets
      bool enabled        = true;
      bool focus          = false;
      bool visible        = true;
      bool highlighted    = false;

      // click-controls
      ButtonType button   = T_PushButton;
      bool       pressed  = false;
      CheckState checked  = Unchecked;

      // editable
      bool       editable = true;
      EchoMode   echo     = Normal;
    };
  }

#endif // WIDGETSTATE_H
