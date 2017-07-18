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

    Tempest::signal<State> onStateChanged;
    Tempest::signal<bool>  onChecked;

    void  setChecked(bool c = true);
    bool  isChecked() const;

    State state() const;
    void  setState(State s);

    void  setTristate(bool y=true);
    bool  isTristate() const;

  protected:
    virtual Tempest::Rect viewRect() const;
    void paintEvent(Tempest::PaintEvent &p);

    void drawBack( Tempest::Painter &p );
    void drawFrame( Tempest::Painter &p );

    void emitClick();

    virtual Size checkIconSize() const;

    void setWidgetState(const WidgetState& s);

  private:
    bool  tristate;
  };
/** @}*/

}

#endif // CHECKBOX_H
