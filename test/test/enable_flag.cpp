#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Widget>
#include <Tempest/Layout>

using namespace testing;
using namespace Tempest;

TEST(all,WidgetEnabled) {
  Widget w[3];

  w[0].layout().add(&w[1]);
  w[1].layout().add(&w[2]);
  w[0].setEnabled(false);

  EXPECT_FALSE(w[2].isEnabled());
  w[1].layout().take(&w[2]);
  EXPECT_TRUE (w[2].isEnabled());
  w[1].layout().add(&w[2]);
  EXPECT_FALSE(w[2].isEnabled());
  w[1].setEnabled(false);
  w[0].setEnabled(true);
  EXPECT_FALSE(w[1].isEnabled());
  w[1].setEnabled(true);
  EXPECT_TRUE (w[2].isEnabled());
  }
