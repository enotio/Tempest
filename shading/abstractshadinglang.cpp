#include "abstractshadinglang.h"

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <Tempest/Texture2d>
#include <Tempest/SystemAPI>
#include <Tempest/Device>

using namespace Tempest;

AbstractShadingLang::VertexShader*
  AbstractShadingLang::createVertexShader( const std::string &fname,
                                           std::string &outputLog ) const {
  return createVertexShaderFromSource( SystemAPI::loadText(fname), outputLog );
  }

AbstractShadingLang::VertexShader*
  AbstractShadingLang::createVertexShader(const std::u16string &fname, std::string &outputLog) const {
  return createVertexShaderFromSource( SystemAPI::loadText(fname), outputLog );
  }

AbstractShadingLang::FragmentShader*
  AbstractShadingLang::createFragmentShader(const std::u16string &fname, std::string & log ) const {
  return createFragmentShaderFromSource( SystemAPI::loadText(fname), log );
  }

std::string AbstractShadingLang::surfaceShader( ShaderType t,
                                                const UiShaderOpt &opt) const {
  bool d = false;
  return surfaceShader(t,opt, d);
  }

AbstractShadingLang::FragmentShader*
  AbstractShadingLang::createFragmentShader(const std::string &fname, std::string &outputLog) const {
  return createFragmentShaderFromSource( SystemAPI::loadText(fname), outputLog );
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

AbstractShadingLang::UiShaderOpt::UiShaderOpt() {
  hasTexture = true;
  vertex     = Tempest::Decl::float2;
  texcoord   = Tempest::Decl::float2;
  color      = Tempest::Decl::float4;
  }
