#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Utility>

using namespace testing;
using namespace Tempest;

TEST(all, Point) {
  Point p0;
  static_assert(sizeof(p0)==sizeof(int)*2,"sizeof(p0)==sizeof(int)*2");
  EXPECT_EQ(p0.x,0);
  EXPECT_EQ(p0.y,0);

  p0+=Point(5,8);
  p0+=Point(1,3);
  EXPECT_EQ(p0.x,6);
  EXPECT_EQ(p0.y,11);
  }

TEST(all, Rect) {
  Rect rect0;
  static_assert(sizeof(Rect)==sizeof(int)*4,"sizeof(Rect)==sizeof(int)*4");
  rect0.w=10;
  rect0.h=20;

  Rect rect1={5,10,10,10};
  Rect ret=rect0.intersected(rect1);
  EXPECT_EQ(ret,Rect(5,10,5,10));

  EXPECT_TRUE (rect0.contains(1,1));
  EXPECT_FALSE(rect0.contains(0,0));
  EXPECT_FALSE(rect0.contains(10,20));
  }
