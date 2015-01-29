#ifndef UIFACTORY_H
#define UIFACTORY_H

#include <string>

namespace Tempest{

class Widget;
class Button;
class Panel;
class LineEdit;
class ListBox;
class ListView;
class StackedWidget;
class ScrollBar;

class UiFactory {
  public:
    virtual ~UiFactory() = default;

    virtual std::u16string tr(const char* src);
    virtual int            metric(int x);

    virtual Widget*        widget();
    virtual Button*        button();
    virtual Panel*         panel();
    virtual LineEdit*      lineEdit();
    virtual ListBox*       listBox();
    virtual ListView*      listView();
    virtual StackedWidget* stackedWidget();
    virtual ScrollBar*     scrollBar();
  };

}

#endif // UIFACTORY_H
