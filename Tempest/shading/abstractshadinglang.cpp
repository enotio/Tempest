#include "abstractshadinglang.h"

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <Tempest/Texture2d>
#include <Tempest/SystemAPI>
#include <Tempest/Device>

using namespace Tempest;

std::string AbstractShadingLang::surfaceShader( ShaderType t,
                                                const UiShaderOpt &opt) const {
  bool d = false;
  return surfaceShader(t,opt, d);
  }

AbstractShadingLang::VertexShader*
    AbstractShadingLang::get( const Tempest::VertexShader& s ){
  return s.data.const_value();
  }

AbstractShadingLang::FragmentShader*
    AbstractShadingLang::get( const Tempest::FragmentShader& s ){
  return s.data.const_value();
  }

AbstractAPI::Texture* AbstractShadingLang::get( const Tempest::Texture2d & t ){
  return t.data.const_value();
  }

AbstractAPI::Texture *AbstractShadingLang::get(const Texture3d &t) {
  return t.data.const_value();
  }

const ShaderInput &AbstractShadingLang::inputOf(const Tempest::VertexShader &s) {
  return s.input;
  }

const ShaderInput &AbstractShadingLang::inputOf(const Tempest::FragmentShader &s) {
  return s.input;
  }


void *AbstractShadingLang::createShader( AbstractShadingLang::ShaderType t,
                                         const std::string &fname,
                                         std::string &outputLog ) const {
  return createShaderFromSource( t, SystemAPI::loadText(fname), outputLog );
  }

void *AbstractShadingLang::createShader(AbstractShadingLang::ShaderType t,
                                         const std::u16string &fname,
                                         std::string &outputLog) const {
  return createShaderFromSource( t, SystemAPI::loadText(fname), outputLog );
  }

GraphicsSubsystem::TessShader*
  AbstractShadingLang::createTessShaderFromSource( const std::string &,
                                                   std::string & ) const {
  return 0;
  }

GraphicsSubsystem::EvalShader*
  AbstractShadingLang::createEvalShaderFromSource( const std::string &,
                                                   std::string & ) const {
  return 0;
  }

AbstractShadingLang::UiShaderOpt::UiShaderOpt() {
  hasTexture = true;
  vertex     = Tempest::Decl::float2;
  texcoord   = Tempest::Decl::float2;
  color      = Tempest::Decl::float4;
  }
