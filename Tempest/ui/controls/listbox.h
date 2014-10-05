#ifndef LISTBOX_H
#define LISTBOX_H

#include <vector>
#include <string>

#include <Tempest/AbstractListBox>

namespace Tempest{

class ListBox : public AbstractListBox {
  public:
    ListBox();

    void setItemList( const std::vector<std::string>    &list );
    void setItemList( const std::vector<std::u16string> &list );
    const std::vector<std::u16string> & items() const;

    Tempest::signal<size_t> onItemSelected;
    Tempest::signal<const std::u16string&> onItemSelectedW;

    void setCurrentItem( size_t i );

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);

  private:
    size_t selected;

    Tempest::Widget *createDropList();
    std::vector<std::u16string> data;

    class ItemBtn;

    void onItem( size_t id );
  };

}

#endif // LISTBOX_H
