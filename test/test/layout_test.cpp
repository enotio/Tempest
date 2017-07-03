#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Widget>

using namespace testing;
using namespace Tempest;

static int moveCount=0;
static int sizeCount=0;

void onMove(int,int){
  moveCount++;
  }

void onSize(int,int){
  sizeCount++;
  }

TEST(all, SetGeometry) {
  moveCount=0;
  sizeCount=0;

  Widget w;
  w.onPositionChange.bind(onMove);
  w.onResize.bind(onSize);

  w.setGeometry(-1,-2,SizePolicy::maxWidgetSize().w+1,100);
  EXPECT_EQ(w.w(),SizePolicy::maxWidgetSize().w);
  EXPECT_EQ(w.h(),100);
  EXPECT_EQ(w.pos(),Point(-1,-2));
  w.setGeometry(-1,-2,200,100);
  EXPECT_EQ(moveCount,1);
  EXPECT_EQ(sizeCount,2);
  }
