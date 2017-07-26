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

TEST(all,TextModel1) {
  TextModel md;
  md.insert(0,u"some text");
  md.clearSteps();

  // delete button
  for(int i=0;i<5;++i)
    md.erase(4,1);
  EXPECT_EQ(md.text(),u"some");

  // backspace button
  for(int i=0;i<4;++i)
    md.erase(md.size()-1,1);
  EXPECT_EQ(md.text(),u"");

  EXPECT_EQ(md.availableUndoSteps(),1);
  md.undo();
  EXPECT_EQ(md.text(),u"some text");
  EXPECT_EQ(md.availableUndoSteps(),0);
  }

TEST(all,TextModel2) {
  const std::u16string inp=u" text";
  TextModel md;
  md.insert(0,u"some");
  md.clearSteps();

  // write text to back
  for(auto& i:inp)
    md.insert(md.size(),i);

  EXPECT_EQ(md.text(),u"some text");

  // delete button
  for(int i=0;i<5;++i)
    md.erase(4,1);
  EXPECT_EQ(md.text(),u"some");

  EXPECT_EQ(md.availableUndoSteps(),3);

  md.undo();
  EXPECT_EQ(md.text(),u"some text");
  md.undo();
  EXPECT_EQ(md.text(),u"some ");
  md.undo();
  EXPECT_EQ(md.text(),u"some");
  EXPECT_EQ(md.availableUndoSteps(),0);
  }

TEST(all,TextModel3) {
  const std::u16string inp=u" long ";
  TextModel md;
  md.insert(0,u"sometext");
  md.clearSteps();

  // write text to center
  for(size_t i=0;i<inp.size();++i)
    md.insert(4+i,inp[i]);

  EXPECT_EQ(md.text(),u"some long text");
  EXPECT_EQ(md.availableUndoSteps(),3);

  md.undo();
  EXPECT_EQ(md.text(),u"some longtext");
  md.undo();
  EXPECT_EQ(md.text(),u"some text");
  md.undo();
  EXPECT_EQ(md.text(),u"sometext");
  EXPECT_EQ(md.availableUndoSteps(),0);
  EXPECT_EQ(md.availableRedoSteps(),3);
  }

TEST(all,TextModel_LimitedUndo) {
  const std::u16string inp=u" long ";
  TextModel md;
  md.setMaxUndoSteps(2);
  md.insert(0,u"sometext");
  md.clearSteps();

  // write text to center
  for(size_t i=0;i<inp.size();++i)
    md.insert(4+i,inp[i]);

  EXPECT_EQ(md.text(),u"some long text");
  EXPECT_EQ(md.availableUndoSteps(),2);

  md.undo();
  EXPECT_EQ(md.text(),u"some longtext");
  md.undo();
  EXPECT_EQ(md.text(),u"some text");

  // undo stack is empty
  EXPECT_FALSE(md.undo());
  EXPECT_EQ(md.text(),u"some text");
  EXPECT_EQ(md.availableUndoSteps(),0);
  EXPECT_EQ(md.availableRedoSteps(),2);

  // change text = clear redo stack
  md.insert(4,u" long");
  EXPECT_EQ(md.text(),u"some long text");
  EXPECT_EQ(md.availableUndoSteps(),1);
  EXPECT_EQ(md.availableRedoSteps(),0);
  }
