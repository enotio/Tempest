#include "fragmentshaderholder.h"

#include <Tempest/FragmentShader>
#include <Tempest/Device>

#include <map>

using namespace Tempest;

struct FragmentShaderHolder::Data {
  std::map< AbstractShadingLang::FragmentShader*, std::string > shaders;
  std::map< AbstractShadingLang::FragmentShader*, std::string > shadersSrc;

  typedef std::map< AbstractShadingLang::FragmentShader*, std::string >::iterator
          iterator;
  };


FragmentShaderHolder::FragmentShaderHolder( Device& d):BaseType(d) {
  data = new Data();
  }

FragmentShaderHolder::~FragmentShaderHolder(){
  delete data;
  }

FragmentShaderHolder::FragmentShaderHolder( const FragmentShaderHolder& h)
                   :BaseType( h.device() ) {
  }

FragmentShader FragmentShaderHolder::loadFromSource(const std::string & src) {
  FragmentShader obj( *this );

  createObjectFromSrc( obj.data.value(), src );
  return obj;
  }

void FragmentShaderHolder::createObjectFromSrc( AbstractShadingLang::FragmentShader *&t,
                                                const std::string &src ) {
  t = device().shadingLang().createFragmentShaderFromSource( src );
  data->shadersSrc[t] = src;
  }

void FragmentShaderHolder::createObject( AbstractShadingLang::FragmentShader*& t,
                                       const std::string & fname ){
  t = device().shadingLang().createFragmentShader( fname );
  data->shaders[t] = fname;
  }

void FragmentShaderHolder::deleteObject( AbstractShadingLang::FragmentShader* t ){
  data->shaders.erase(t);
  reset(t);
  }

void FragmentShaderHolder::reset( AbstractShadingLang::FragmentShader* t ){
  device().shadingLang().deleteFragmentShader(t);
  }

AbstractShadingLang::FragmentShader*
      FragmentShaderHolder::restore( AbstractShadingLang::FragmentShader* t ){
  Data::iterator i = data->shaders.find(t);

  if( i!=data->shaders.end() ){
    std::string fname = data->shaders[t];
    data->shaders.erase(t);

    createObject( t, fname );
    return t;
    } else {
    std::string src = data->shadersSrc[t];
    data->shadersSrc.erase(t);

    createObjectFromSrc( t, src );
    return t;
    }
  }

AbstractShadingLang::FragmentShader*
      FragmentShaderHolder::copy( AbstractShadingLang::FragmentShader* t ){
  Data::iterator i = data->shaders.find(t);
  AbstractShadingLang::FragmentShader* ret = 0;

  if( i!=data->shaders.end() ){
    createObject( ret, data->shaders[t] );
    return ret;
    } else {
    createObjectFromSrc( ret, data->shadersSrc[t] );
    return ret;
    }
  }
