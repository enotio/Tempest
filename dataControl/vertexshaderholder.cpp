#include "vertexshaderholder.h"

#include <Tempest/VertexShader>
#include <Tempest/Device>

#include <map>

using namespace Tempest;

struct VertexShaderHolder::Data {
  std::map< AbstractShadingLang::VertexShader*, std::string > shaders,    restore;
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

void VertexShaderHolder::createObjectFromSrc( AbstractShadingLang::VertexShader *&t,
                                              const std::string &src ) {
  t = device().shadingLang().createVertexShaderFromSource( src );
  data->shadersSrc[t] = src;
  }

VertexShaderHolder::VertexShaderHolder( const VertexShaderHolder& h)
                   :BaseType( h.device() ) {
  }

void VertexShaderHolder::createObject( AbstractShadingLang::VertexShader*& t,
                                       const std::string & fname ){
  t = device().shadingLang().createVertexShader( fname );
  data->shaders[t] = fname;
  }

void VertexShaderHolder::deleteObject( AbstractShadingLang::VertexShader* t ){
  data->shaders.erase(t);
  reset(t);
  }

void VertexShaderHolder::reset( AbstractShadingLang::VertexShader* t ){
  Data::iterator i = data->shaders.find(t);

  if( i!=data->shaders.end() ){
    data->restore[t] = i->second;
    data->shaders.erase(i);
    } else {
    data->restoreSrc[t] = data->shadersSrc[t];
    data->shadersSrc.erase(t);
    }

  device().shadingLang().deleteVertexShader(t);
  }

AbstractShadingLang::VertexShader*
      VertexShaderHolder::restore( AbstractShadingLang::VertexShader* t ){
  Data::iterator i = data->restore.find(t);

  if( i!=data->restore.end() ){
    std::string fname = data->restore[t];
    data->restore.erase(i);

    createObject( t, fname );
    return t;
    } else {
    std::string src = data->restoreSrc[t];
    data->restoreSrc.erase(t);

    createObjectFromSrc( t, src );
    return t;
    }
  }

AbstractShadingLang::VertexShader*
      VertexShaderHolder::copy( AbstractShadingLang::VertexShader* t ){
  Data::iterator i = data->shaders.find(t);
  AbstractShadingLang::VertexShader* ret = 0;

  if( i!=data->shaders.end() ){
    createObject( ret, data->shaders[t] );
    return ret;
    } else {
    createObjectFromSrc( ret, data->shadersSrc[t] );
    return ret;
    }
  }
