#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <Tempest/Widget>

namespace Tempest {

/** \addtogroup GUI
 *  @{
 */
class StackedWidget : public Widget {
  public:
    StackedWidget();
    ~StackedWidget();

    void    addPage(Widget* w);
    void    delPage(Widget* w);
    Widget* takePage(Widget*);

    Widget &page(size_t num);
    const Widget &page(size_t num) const;
    size_t        pagesCount() const;
    size_t        currentPage() const;

    void setCurrentPage(size_t num);

    Tempest::signal<size_t> onPageChanged;

    static const size_t noPage;
  private:
    struct Layout;

    using Widget::layout;
    void setLayout(Tempest::Layout*);

    std::vector<Widget*> pages;
    size_t current;
  };
/** @}*/

}

#endif // STACKEDWIDGET_H
