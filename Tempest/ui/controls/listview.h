#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <Tempest/Widget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>

#include <unordered_set>

namespace Tempest {

class ListView : public Widget {
  public:
    ListView(ListDelegate& delegate);

  private:
    struct Layout : Tempest::Layout {
      Layout(ListDelegate& delegate, Tempest::Orientation ori);
      ~Layout();

      void    applyLayout();
      Widget* widget(size_t i);

      Tempest::Orientation ori;
      size_t               placedStart;
      std::vector<Widget*> placed;
      ListDelegate& delegate;

      void invalidate();
      bool busy;
      };

    using Widget::layout;
  };

}

#endif // LISTVIEW_H
