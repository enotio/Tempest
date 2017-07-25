#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/Pixmap>
#include <Tempest/IODevice>
#include <Tempest/Buffer>

using namespace testing;
using namespace Tempest;

static uint64_t hash(Pixmap& p){
  uint32_t vr=0,vg=0,vb=0,va=0;
  for(int i=0;i<p.width();++i)
    for(int r=0;r<p.height();++r){
      auto px=p.at(i,r);
      vr+=px.r;
      vg+=px.g;
      vb+=px.b;
      va+=px.a;
      }
  uint64_t combo=0,ret=0;
  char*    pret =reinterpret_cast<char*>(&combo);
  memcpy(pret  ,&vr,4);
  memcpy(pret+4,&vg,4);
  ret+=combo;
  memcpy(pret  ,&vb,4);
  memcpy(pret+4,&va,4);
  ret+=combo;

  return ret;
  }

TEST(all, Pixmap0) {
  Pixmap pm  (64,64,true);
  Pixmap mark(8,6,false);

  ASSERT_EQ(pm.width(), 64);
  ASSERT_EQ(pm.height(),64);

  Pixmap::Pixel px={255,0,0,255};
  mark.fill(px);

  Tempest::PixEditor p( pm );
  p.copy(0,0,mark);
  EXPECT_EQ(hash(pm),52570399715280ull);

  std::vector<char> mem;
  BufferWriter out(mem);
  EXPECT_TRUE(pm.save(out,"PNG"));

  Pixmap other;
  BufferReader inp(mem);
  EXPECT_TRUE(other.load(inp));

  EXPECT_EQ(hash(other),hash(pm));
  }

TEST(all, Pixmap1) {
  Pixmap pm  (64,64,true);
  Pixmap mark(8,6,true);

  ASSERT_EQ(pm.width(), 64);
  ASSERT_EQ(pm.height(),64);

  Pixmap::Pixel px={255,0,0,255};
  mark.fill(px);

  Tempest::PixEditor p( pm );
  p.copy(0,0,mark);
  EXPECT_EQ(hash(pm),52570399715280ull);

  std::vector<char> mem;
  BufferWriter out(mem);
  EXPECT_TRUE(pm.save(out,"PNG"));

  Pixmap other;
  BufferReader inp(mem);
  EXPECT_TRUE(other.load(inp));

  EXPECT_EQ(hash(other),hash(pm));
  }

