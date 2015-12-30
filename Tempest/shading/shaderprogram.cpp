#include "shaderprogram.h"

#include <Tempest/Device>

using namespace Tempest;

ShaderProgram::ShaderProgram():data( ShaderProgramHolder::ImplManip(0) ){
  ubo.reset( new std::vector<AbstractShadingLang::UBO>() );
  }

ShaderProgram::ShaderProgram(const ShaderProgram &p)
  :data(p.data), logv(p.logv) {
  ubo.reset( new std::vector<AbstractShadingLang::UBO>(*p.ubo) );
  }

ShaderProgram &ShaderProgram::operator =(const ShaderProgram & p) {
  data = p.data;
  logv = p.logv;
  *ubo = *p.ubo;
  return *this;
  }

ShaderProgram::~ShaderProgram() {
  }

ShaderProgram::ShaderProgram(AbstractHolder< ShaderProgram,
                                              GraphicsSubsystem::ProgramObject> &h)
  :data( h.makeManip() ) {
  ubo.reset( new std::vector<AbstractShadingLang::UBO>() );
  }

bool ShaderProgram::isValid() const {
  return data.const_value()!=0;
  }

const std::string &ShaderProgram::log() const {
  return logv;
  }

void ShaderProgram::implSetUniform( AbstractShadingLang::UBO& ux,
                                    const char *ubo,
                                    const UniformDeclaration &decl) {
  AbstractShadingLang::assignUniformBuffer(ux,ubo,decl);
  }
