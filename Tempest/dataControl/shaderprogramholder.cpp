#include "shaderprogramholder.h"
#include <unordered_map>

#include <Tempest/Device>

using namespace Tempest;

/// \cond HIDDEN_SYMBOLS
struct ShaderProgramHolder::Data {
  std::unordered_map<AbstractAPI::ProgramObject*,ShaderProgram::Source> sh;
  };
/// \endcond

ShaderProgramHolder::ShaderProgramHolder(Device &d):BaseType(d){
  data = new Data();
  }

ShaderProgramHolder::~ShaderProgramHolder() {
  T_WARNING_X(data->sh.size()==0,"Not all shaders was deleted");
  delete data;
  }

ShaderProgram ShaderProgramHolder::compile(const ShaderProgram::Source &src) {
  Tempest::ShaderProgram obj( *this );

  AbstractAPI::ProgramObject* t = device().createShaderFromSource( src, obj.logv );
  if(t){
    obj.data.value() = t;
    data->sh[t] = src;
    }

  return obj;
  }

ShaderProgram ShaderProgramHolder::load(const ShaderProgramHolder::Source &files) {
  Source src;
  src.vs = SystemAPI::loadText(files.vs);
  src.fs = SystemAPI::loadText(files.fs);
  if(files.ts.size()>0)
    src.ts = SystemAPI::loadText(files.ts);
  if(files.es.size()>0)
    src.es = SystemAPI::loadText(files.es);
  return compile(src);
  }

ShaderProgram ShaderProgramHolder::surfaceShader(const AbstractShadingLang::UiShaderOpt &opt,
                                                 bool &hasHalfPixelOffset){
  return compile( this->device().surfaceShader(opt,hasHalfPixelOffset) );
  }

void ShaderProgramHolder::deleteObject(GraphicsSubsystem::ProgramObject *t) {
  data->sh.erase(t);
  reset(t);
  }

void ShaderProgramHolder::reset(GraphicsSubsystem::ProgramObject *t) {
  this->device().deleteShader(t);
  }

GraphicsSubsystem::ProgramObject*
  ShaderProgramHolder::restore(GraphicsSubsystem::ProgramObject *t) {
  auto i = data->sh.find(t);

  if( i!=data->sh.end() ){
    std::string log;
    return device().createShaderFromSource( i->second, log );
    } else {
    return 0;
    }
  }

GraphicsSubsystem::ProgramObject*
  ShaderProgramHolder::copy(GraphicsSubsystem::ProgramObject *t) {
  return t;
  }
