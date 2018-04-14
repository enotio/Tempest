#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <array>

#include <Tempest/UniformDeclaration>
#include <Tempest/AbstractShadingLang>
#include <Tempest/Matrix4x4>

using namespace testing;
using namespace Tempest;

TEST(Uniforms, floats) {
  struct Cpp {
    float               x=1;
    std::array<float,2> y={{2,3}};
    std::array<float,3> z={{4,5,6}};
    };
  Cpp                      uin;
  AbstractShadingLang::UBO uout;

  UniformDeclaration udecl;
  udecl.add("x",Decl::float1)
       .add("y",Decl::float2)
       .add("z",Decl::float3);
  AbstractShadingLang::assignUniformBuffer(uout,reinterpret_cast<const char*>(&uin),udecl);

  const float* dest=reinterpret_cast<const float*>(uout.data.data());
  EXPECT_EQ(dest[0],1.f);
  EXPECT_EQ(dest[2],2.f);
  EXPECT_EQ(dest[3],3.f);
  EXPECT_EQ(dest[4],4.f);
  EXPECT_EQ(dest[5],5.f);
  EXPECT_EQ(dest[6],6.f);
  }

TEST(Uniforms, matrix) {
  struct Cpp {
    float               x=8;
    Tempest::Matrix4x4  y;
    float               z=9;
    };
  Cpp                      uin;
  AbstractShadingLang::UBO uout;

  UniformDeclaration udecl;
  udecl.add("x",Decl::float1)
       .add("y",Decl::Matrix4x4)
       .add("z",Decl::float1);
  AbstractShadingLang::assignUniformBuffer(uout,reinterpret_cast<const char*>(&uin),udecl);

  const float* dest=reinterpret_cast<const float*>(uout.data.data());
  EXPECT_EQ(dest[0], 8.f);
  EXPECT_EQ(dest[2], 1.f);
  EXPECT_EQ(dest[18],9.f);
  }
