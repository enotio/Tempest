#include "vertexshaderholder.h"

#include <Tempest/VertexShader>
#include <Tempest/Device>
#include <Tempest/SystemAPI>

#include <map>

using namespace Tempest;

struct VertexShaderHolder::Data {
  // std::map< AbstractShadingLang::VertexShader*, std::string > shaders,    restore;
  std::map< AbstractShadingLang::VertexShader*, std::string > shadersSrc, restoreSrc;

  typedef std::map< AbstractShadingLang::VertexShader*, std::string >::iterator
          iterator;
  };

VertexShaderHolder::VertexShaderHolder( Device& d):BaseType(d) {
  data = new Data();
  }

VertexShaderHolder::~VertexShaderHolder(){
  delete data;
  }

VertexShader VertexShaderHolder::loadFromSource(const std::string & src) {
  VertexShader obj( *this );

  createObjectFromSrc( obj.data.value(), src );
  return obj;
  }

void VertexShaderHolder::createObjectFromSrc(AbstractShadingLang::VertexShader *&t,
                                              const std::string &src ) {
  t = device().shadingLang().createVertexShaderFromSource( src );
  data->shadersSrc[t] = src;
  }

VertexShaderHolder::VertexShaderHolder( const VertexShaderHolder& h)
                   :BaseType( h.device() ) {
  }

void VertexShaderHolder::createObject(AbstractShadingLang::VertexShader*& t,
                                       const std::wstring &fname ){
  createObjectFromSrc( t, SystemAPI::loadText(fname) );
  }

void VertexShaderHolder::deleteObject( AbstractShadingLang::VertexShader* t ){
  if( t==0 )
    return;

  //data->shaders.erase(t);
  reset(t);
  }

void VertexShaderHolder::reset( AbstractShadingLang::VertexShader* t ){
  if( t==0 )
    return;

  data->restoreSrc[t] = data->shadersSrc[t];
  data->shadersSrc.erase(t);

  device().shadingLang().deleteVertexShader(t);
  }

AbstractShadingLang::VertexShader*
      VertexShaderHolder::restore( AbstractShadingLang::VertexShader* t ){
  if( t==0 )
    return 0;

  std::string src = data->restoreSrc[t];
  data->restoreSrc.erase(t);

  createObjectFromSrc( t, src );
  return t;
  }

AbstractShadingLang::VertexShader*
      VertexShaderHolder::copy( AbstractShadingLang::VertexShader* t ){
  if( t==0 )
    return 0;

  AbstractShadingLang::VertexShader* ret = 0;
  createObjectFromSrc( ret, data->shadersSrc[t] );
  return ret;
  }
