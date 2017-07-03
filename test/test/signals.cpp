#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <Tempest/signal>
#include <string>

using namespace testing;
using namespace Tempest;

static std::string fooCalls;

void sortStr(std::string& s){
  std::sort(s.begin(),s.end());
  }

TEST(all, Signal) {
  struct SigTest {
    char ident='a';
    void foo(){
      fooCalls+=ident;
      }
    };

  fooCalls.clear();
  SigTest  obj[3];
  obj[0].ident='a';
  obj[1].ident='b';
  obj[2].ident='c';

  Tempest::signal<> onTest;
  onTest.bind(obj[0],&SigTest::foo);
  onTest.bind(obj[1],&SigTest::foo);
  onTest.bind(obj[2],&SigTest::foo);

  onTest();
  sortStr(fooCalls);
  EXPECT_EQ(fooCalls,"abc");
  fooCalls.clear();

  auto copySig=onTest;

  onTest.ubind(obj[1],&SigTest::foo);
  onTest();
  sortStr(fooCalls);
  EXPECT_EQ(fooCalls,"ac");
  fooCalls.clear();

  copySig();
  sortStr(fooCalls);
  EXPECT_EQ(fooCalls,"abc");
  fooCalls.clear();
  }

TEST(all, SignalUbind) {
  struct SigTest {
    Tempest::signal<>* onTest=nullptr;
    char ident='a';

    void foo(){
      fooCalls+=ident;
      onTest->ubind(this,&SigTest::foo);
      }
    };
  fooCalls.clear();
  SigTest  obj[3];
  obj[0].ident='a';
  obj[1].ident='b';
  obj[2].ident='c';

  Tempest::signal<> onTest;
  onTest.bind(obj[0],&SigTest::foo);
  onTest.bind(obj[1],&SigTest::foo);
  onTest.bind(obj[2],&SigTest::foo);

  for(auto& i:obj)
    i.onTest=&onTest;

  onTest();
  sortStr(fooCalls);
  EXPECT_EQ(fooCalls,"abc");

  onTest();
  sortStr(fooCalls);
  EXPECT_EQ(fooCalls,"abc");
  fooCalls.clear();
  }
