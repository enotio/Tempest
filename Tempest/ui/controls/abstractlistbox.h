#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include <Tempest/Layout>
#include <Tempest/Window>

namespace Tempest{

class AbstractListBox : public Widget {
  public:
    AbstractListBox();
    ~AbstractListBox();

  protected:
    void showList();

    bool needToShow;
    uint64_t lastRM;

    struct Overlay:Tempest::WindowOverlay {
      ~Overlay();
      void mouseDownEvent( Tempest::MouseEvent& e );

      AbstractListBox* owner;
      };

    virtual Tempest::Widget *createDropList() = 0;
    void close();

    Overlay * dropListLayer();

    struct WrapLayout: Layout{
      void applyLayout();
      };
  private:
    Overlay * overlay;
  };

}

#endif // ABSTRACTLISTBOX_H
