#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <Tempest/Button>

namespace Tempest{

class CheckBox: public Button {
  public:
    CheckBox();

    Tempest::signal<bool> onChecked;

    void setChecked(bool c = true);
    bool isClicked() const;

  protected:
    virtual Tempest::Rect viewRect() const;
    void paintEvent(Tempest::PaintEvent &p);

    void drawBack( Tempest::Painter &p );
    void drawFrame( Tempest::Painter &p );

    void emitClick();

    virtual Size checkIconSize() const;

  private:
    bool state;
  };

}

#endif // CHECKBOX_H
