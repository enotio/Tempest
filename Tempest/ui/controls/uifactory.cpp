#include "uifactory.h"

#include <Tempest/Button>
#include <Tempest/Panel>
#include <Tempest/LineEdit>
#include <Tempest/ListBox>
#include <Tempest/ListView>
#include <Tempest/StackedWidget>

using namespace Tempest;

std::u16string UiFactory::tr(const char *src) {
  return SystemAPI::toUtf16(src);
  }

int UiFactory::metric(int x) {
  return x;
  }

Widget *UiFactory::widget() {
  return new Widget();
  }

Button *UiFactory::button() {
  return new Button();
  }

Panel *UiFactory::panel() {
  return new Panel();
  }

LineEdit *UiFactory::lineEdit() {
  return new LineEdit();
  }

ListBox *UiFactory::listBox() {
  return new ListBox();
  }

ListView *UiFactory::listView() {
  return new ListView();
  }

StackedWidget* UiFactory::stackedWidget() {
  return new StackedWidget();
  }
