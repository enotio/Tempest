#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include <Tempest/Button>
#include <Tempest/Window>

namespace Tempest{

class AbstractListBox : public Button {
  public:
    AbstractListBox();
    ~AbstractListBox();

  private:
    void showList();

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    bool needToShow;
    uint64_t lastRM;

    struct Overlay:Tempest::WindowOverlay {
      ~Overlay();
      void mouseDownEvent( Tempest::MouseEvent& e );

      AbstractListBox* owner;
      };
    Overlay * overlay;

    virtual Tempest::Widget *createDropList();
    void close();
  };

}

#endif // ABSTRACTLISTBOX_H
