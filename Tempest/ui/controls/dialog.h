#ifndef DIALOG_H
#define DIALOG_H

#include <Tempest/Panel>

namespace Tempest {
  class WindowOverlay;

class Dialog : public Panel {
  public:
    Dialog();
    ~Dialog();

    int  exec();
    void close();

    void setModal(bool m);
    bool isModal() const;

  protected:
    void closeEvent(Tempest::CloseEvent& e);
    virtual void paintShadow(PaintEvent& e);

  private:
    struct Overlay;
    struct LayShadow;

    Overlay* owner_ov;
  };

}
#endif // DIALOG_H
