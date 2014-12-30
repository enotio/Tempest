#ifndef LISTBOX_H
#define LISTBOX_H

#include <vector>
#include <string>

#include <Tempest/AbstractListBox>

namespace Tempest{

class ListDelegate;

class ListBox : public AbstractListBox {
  public:
    ListBox();
    ~ListBox();

    template<class D>
    void setDelegate(const D& d){
      removeDelegate();

      delegate.reset(new D(d));
      setupView(size_t(-1));
      seupDelegateCallback();
      }

    void removeDelegate();

    Tempest::signal<size_t> onItemSelected;

    void   setCurrentItem( size_t i );
    size_t currentItem() const;

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    using AbstractListBox::layout;

    void setupView(size_t oldSelected);

  private:
    size_t selected;
    bool dropListEnabled;
    std::shared_ptr<ListDelegate> delegate;

    Tempest::Widget *createDropList();
    Tempest::Widget *view;

    class ItemBtn;

    void onItem(size_t id , Widget *view);
    void selectItem( size_t id );

    void seupDelegateCallback();

    struct ProxyDelegate;
  };

}

#endif // LISTBOX_H
