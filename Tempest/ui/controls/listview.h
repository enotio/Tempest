#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <Tempest/Widget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>

#include <unordered_set>
#include <memory>

namespace Tempest {

class ListView : public Widget {
  public:
    ListView(Tempest::Orientation ori = Tempest::Vertical);
    ~ListView();

    template<class D>
    void setDelegate(const D& d){
      removeDelegate();

      delegate.reset(new D(d));
      delegate->onItemSelected.bind(onItemSelected);
      setLayout(new Layout(*delegate,orientation));
      }

    void removeDelegate();

    void setOrientation(Tempest::Orientation ori);

    Tempest::signal<size_t> onItemSelected;

  private:
    Tempest::Orientation          orientation;
    std::unique_ptr<ListDelegate> delegate;

    struct Layout : Tempest::LinearLayout {
      Layout(ListDelegate& delegate, Tempest::Orientation ori);
      ~Layout();

      void applyLayout();
      void invalidate();

      ListDelegate& delegate;
      bool          busy;
      void          removeAll();
      };

    using Widget::layout;
    using Widget::setLayout;
  };

}

#endif // LISTVIEW_H
