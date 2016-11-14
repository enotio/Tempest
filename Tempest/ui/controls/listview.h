#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <Tempest/Widget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>

#include <unordered_set>
#include <memory>

namespace Tempest {

/** \addtogroup GUI
 *  @{
 */
class ListView : public Widget {
  public:
    ListView(Tempest::Orientation ori = Tempest::Vertical);
    ~ListView();

    template<class D>
    void setDelegate(const D& d){
      removeDelegate();

      delegate.reset(new D(d));
      delegate->onItemSelected.bind(onItemSelected);
      lay = new Layout(*this,*delegate,orientation,defaultRole);
      setLayout(lay);
      }

    void removeDelegate();

    void invalidateView();
    void updateView();

    void setOrientation(Tempest::Orientation ori);

    void setDefaultItemRole(ListDelegate::Role role);
    ListDelegate::Role defaultItemRole() const;

    Tempest::signal<size_t> onItemSelected;
    Tempest::signal<>       onItemListChanged;

  private:
    Tempest::Orientation          orientation;
    std::unique_ptr<ListDelegate> delegate;
    ListDelegate::Role defaultRole = ListDelegate::R_ListItem;

    struct Layout : Tempest::LinearLayout {
      Layout( ListView& view,
              ListDelegate& delegate,
              Tempest::Orientation ori,
              ListDelegate::Role& defaultRole );
      ~Layout();

      void applyLayout();
      void invalidate();
      void update();

      ListView&           view;
      ListDelegate&       delegate;
      bool                busy;
      ListDelegate::Role& defaultRole;
      void                removeAll();
      };

    Layout* lay;

    using Widget::layout;
    using Widget::setLayout;

    void removeAll();
  };
/** @}*/

}

#endif // LISTVIEW_H
