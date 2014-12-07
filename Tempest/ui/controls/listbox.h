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

    void setCurrentItem( size_t i );

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    using AbstractListBox::layout;

    void setupView(size_t oldSelected);

  private:
    size_t selected;
    bool dropListEnabled;
    ListDelegate& delegate;

    Tempest::Widget *createDropList();
    Tempest::Widget *view;

    class ItemBtn;

    void onItem(size_t id , Widget *view);
    void selectItem( size_t id );
  };

}

#endif // LISTBOX_H
