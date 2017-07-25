#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <Tempest/Button>

namespace Tempest{

/** \addtogroup GUI
 *  @{
 */
class CheckBox: public Button {
  public:
    CheckBox();

    using State=Tempest::WidgetState::CheckState;

    static constexpr State Unchecked       =State::Unchecked;
    static constexpr State Checked         =State::Checked;
    static constexpr State PartiallyChecked=State::PartiallyChecked;

    Tempest::signal<State> onStateChanged;
    Tempest::signal<bool>  onChecked;

    void  setChecked(bool c = true);
    bool  isChecked() const;

    State state() const;
    void  setState(State s);

    void  setTristate(bool y=true);
    bool  isTristate() const;

  protected:
    void paintEvent(Tempest::PaintEvent &p);
    void emitClick();
    void setWidgetState(const WidgetState& s);

  private:
    bool tristate = false;
  };
/** @}*/

}

#endif // CHECKBOX_H
