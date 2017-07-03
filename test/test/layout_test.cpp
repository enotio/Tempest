#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Widget>

using namespace testing;
using namespace Tempest;

TEST(all, SetGeometry) {
  Widget w;
  w.setGeometry(-1,-2,SizePolicy::maxWidgetSize().w+1,100);
  EXPECT_LE(w.w(),SizePolicy::maxWidgetSize().w);
  }
