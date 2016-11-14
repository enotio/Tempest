#ifndef UIFACTORY_H
#define UIFACTORY_H

#include <string>
#include <Tempest/Widget>

namespace Tempest {

class Widget;
class Button;
class Panel;
class LineEdit;
class Label;
class ListBox;
class ListView;
class StackedWidget;
class ScrollBar;
class ScrollWidget;
class CheckBox;

/** \addtogroup GUI
 *  @{
 */
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
    virtual Label*         label();
    virtual ListView*      listView();
    virtual StackedWidget* stackedWidget();
    virtual ScrollBar*     scrollBar(Tempest::Orientation ori);
    virtual ScrollWidget*  scrollWidget();
    virtual CheckBox*      checkBox();
  };
/** @}*/

}

#endif // UIFACTORY_H
