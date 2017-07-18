#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/CheckBox>

using namespace testing;
using namespace Tempest;

static int ckCount=0;
static int stCount=0;

static void checks(bool){
  ckCount++;
  }

static void state(CheckBox::State){
  stCount++;
  }

static void clearGlobals(){
  ckCount=0;
  stCount=0;
  }

TEST(all,CheckboxState) {
  clearGlobals();

  CheckBox box;
  box.onChecked.bind(checks);
  box.onStateChanged.bind(state);

  box.setChecked(true);
  EXPECT_TRUE(box.isChecked());
  EXPECT_EQ(ckCount,1);
  EXPECT_EQ(stCount,1);

  box.setState(CheckBox::State::PartiallyChecked);
  EXPECT_FALSE(box.isChecked());
  EXPECT_EQ(ckCount,2);
  EXPECT_EQ(stCount,2);

  box.setState(CheckBox::State::Unchecked);
  EXPECT_FALSE(box.isChecked());
  EXPECT_EQ(ckCount,2);
  EXPECT_EQ(stCount,2);
  }

TEST(all,TriboxState) {
  clearGlobals();

  CheckBox box;
  box.setTristate(true);
  box.onChecked.bind(checks);
  box.onStateChanged.bind(state);

  box.setChecked(true);
  EXPECT_TRUE(box.isChecked());
  EXPECT_EQ(ckCount,1);
  EXPECT_EQ(stCount,1);

  box.setState(CheckBox::State::PartiallyChecked);
  EXPECT_FALSE(box.isChecked());
  EXPECT_EQ(ckCount,2);
  EXPECT_EQ(stCount,2);

  box.setState(CheckBox::State::Unchecked);
  EXPECT_FALSE(box.isChecked());
  EXPECT_EQ(ckCount,2);
  EXPECT_EQ(stCount,3);
  }
