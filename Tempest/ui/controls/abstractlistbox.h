#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include <Tempest/Layout>
#include <Tempest/Window>
#include <Tempest/ListDelegate>

namespace Tempest{

/** \addtogroup GUI
 *  @{
 */
class AbstractListBox : public Widget {
  public:
    AbstractListBox();
    ~AbstractListBox();

  protected:
    void showList();

    struct Overlay:Tempest::WindowOverlay {
      ~Overlay();
      void mouseDownEvent( Tempest::MouseEvent& e );

      AbstractListBox* owner;
      };

    virtual Tempest::Widget *createDropList() = 0;
    virtual Tempest::Button *createItemButton(ListDelegate &owner, size_t pos, Tempest::ListDelegate::Role r) = 0;
    void close();

    Overlay * dropListLayer();

    struct WrapLayout: Layout{
      void applyLayout();
      };

  private:
    Overlay * overlay;    
    void setLayout(Tempest::Layout* l);
  };
/** @}*/

}

#endif // ABSTRACTLISTBOX_H
