#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Pixmap>

using namespace testing;
using namespace Tempest;

TEST(all, Pixmap) {
  Pixmap pm  (64,64,true);
  Pixmap mark(8,6,false);

  ASSERT_EQ(pm.width(), 64);
  ASSERT_EQ(pm.height(),64);


  Tempest::PixEditor p( pm );
  p.copy(0,0,mark);
  }
