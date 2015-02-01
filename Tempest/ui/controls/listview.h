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
      lay = new Layout(*this,*delegate,orientation);
      setLayout(lay);
      }

    void removeDelegate();

    void invalidateView();
    void updateView();

    void setOrientation(Tempest::Orientation ori);

    Tempest::signal<size_t> onItemSelected;
    Tempest::signal<>       onItemListChanged;

  private:
    Tempest::Orientation          orientation;
    std::unique_ptr<ListDelegate> delegate;

    struct Layout : Tempest::LinearLayout {
      Layout(ListView& view, ListDelegate& delegate, Tempest::Orientation ori);
      ~Layout();

      void applyLayout();
      void invalidate();
      void update();

      ListView&     view;
      ListDelegate& delegate;
      bool          busy;
      void          removeAll();
      };

    Layout* lay;

    using Widget::layout;
    using Widget::setLayout;

    void removeAll();
  };

}

#endif // LISTVIEW_H
