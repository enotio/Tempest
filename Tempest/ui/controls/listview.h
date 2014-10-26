#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <Tempest/Widget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>

#include <unordered_set>

namespace Tempest {

class ListView : public Widget {
  public:
    ListView(ListDelegate& delegate,Tempest::Orientation ori = Tempest::Vertical);

    void setOrientation(Tempest::Orientation ori);

    Tempest::signal<size_t> onItemSelected;

  private:
    Tempest::Orientation orientation;

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
  };

}

#endif // LISTVIEW_H
