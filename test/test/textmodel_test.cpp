#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/TextModel>

using namespace testing;
using namespace Tempest;

TEST(all,TextModel0) {
  TextModel md;
  md.insert(0,u"some text");
  EXPECT_EQ(md.text(),u"some text");
  md.insert(4,u" long");
  EXPECT_EQ(md.text(),u"some long text");
  md.undo();
  EXPECT_EQ(md.text(),u"some text");
  md.undo();
  EXPECT_EQ(md.text(),u"");
  }
