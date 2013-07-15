#include "abstractshadinglang.h"

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <Tempest/Texture2d>
#include <Tempest/SystemAPI>

using namespace Tempest;

AbstractShadingLang::VertexShader*
  AbstractShadingLang::createVertexShader(const std::string &fname) const {
  return createVertexShaderFromSource( SystemAPI::loadText(fname) );
  }

AbstractShadingLang::VertexShader*
  AbstractShadingLang::createVertexShader(const std::wstring &fname) const {
  return createVertexShaderFromSource( SystemAPI::loadText(fname) );
  }

AbstractShadingLang::FragmentShader*
  AbstractShadingLang::createFragmentShader(const std::wstring &fname) const {
  return createFragmentShaderFromSource( SystemAPI::loadText(fname) );
  }

AbstractShadingLang::FragmentShader*
  AbstractShadingLang::createFragmentShader(const std::string &fname) const {
  return createFragmentShaderFromSource( SystemAPI::loadText(fname) );
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

const ShaderInput &AbstractShadingLang::inputOf(const Tempest::VertexShader &s) {
  return s.input;
  }

const ShaderInput &AbstractShadingLang::inputOf(const Tempest::FragmentShader &s) {
  return s.input;
  }
