#include "uifactory.h"

#include <Tempest/Button>
#include <Tempest/Panel>
#include <Tempest/LineEdit>

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
