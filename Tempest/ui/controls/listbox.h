#ifndef LISTBOX_H
#define LISTBOX_H

#include <vector>
#include <string>

#include <Tempest/AbstractListBox>

namespace Tempest{

class ListDelegate;

class ListBox : public AbstractListBox {
  public:
    ListBox(ListDelegate& delegate);

    Tempest::signal<size_t> onItemSelected;
    Tempest::signal<const std::u16string&> onItemSelectedW;

    void setCurrentItem( size_t i );

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);

  private:
    size_t selected;
    ListDelegate& delegate;

    Tempest::Widget *createDropList();

    class ItemBtn;

    void onItem( size_t id );
  };

}

#endif // LISTBOX_H
