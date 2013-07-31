#include "fragmentshaderholder.h"

#include <Tempest/FragmentShader>
#include <Tempest/Device>
#include <Tempest/SystemAPI>

#include <map>
/*
using namespace Tempest;

struct FragmentShaderHolder::Data {
  //std::map< AbstractShadingLang::FragmentShader*, std::string > shaders;
  std::map< AbstractShadingLang::FragmentShader*, std::string > shadersSrc;

  typedef std::map< AbstractShadingLang::FragmentShader*, std::string >::iterator
          iterator;

  //std::map< AbstractShadingLang::FragmentShader*, std::string > restore;
  std::map< AbstractShadingLang::FragmentShader*, std::string > restoreSrc;
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

  createObjectFromSrc( obj, obj.data.value(), src );
  return obj;
  }

void FragmentShaderHolder::createObjectFromSrc( FragmentShader &obj,
                                                AbstractShadingLang::FragmentShader *&t,
                                                const std::string &src) {
  t = device().shadingLang().createFragmentShaderFromSource( src, obj.logv );
  if( t )
    data->shadersSrc[t] = src;
  }

void FragmentShaderHolder::createObjectFromSrc( AbstractShadingLang::FragmentShader *&t,
                                                const std::string &src ) {
  std::string log;
  t = device().shadingLang().createFragmentShaderFromSource( src, log );
  if( t )
    data->shadersSrc[t] = src;
  }

void FragmentShaderHolder::createObject( FragmentShader &obj,
                                         AbstractShadingLang::FragmentShader*& t,
                                         const std::wstring &fname){
  createObjectFromSrc( obj, t, SystemAPI::loadText(fname) );
  }

void FragmentShaderHolder::createObject( AbstractShadingLang::FragmentShader *&t,
                                         const std::wstring &f) {
  createObjectFromSrc(t, SystemAPI::loadText(f));
  }

void FragmentShaderHolder::deleteObject( AbstractShadingLang::FragmentShader* t ){
  if( t==0 )
    return;

  reset(t);
  }

void FragmentShaderHolder::reset( AbstractShadingLang::FragmentShader* t ){
  if( t==0 )
    return;

  data->restoreSrc[t] = data->shadersSrc[t];
  data->shadersSrc.erase(t);

  device().shadingLang().deleteFragmentShader(t);
  }

AbstractShadingLang::FragmentShader*
      FragmentShaderHolder::restore( AbstractShadingLang::FragmentShader* t ){
  if( t==0 )
    return 0;

  std::string src = data->restoreSrc[t];
  data->restoreSrc.erase(t);

  createObjectFromSrc( t, src );
  return t;
  }

AbstractShadingLang::FragmentShader*
      FragmentShaderHolder::copy( AbstractShadingLang::FragmentShader* t ){
  if( t==0 )
    return 0;

  AbstractShadingLang::FragmentShader* ret = 0;

  createObjectFromSrc( ret, data->shadersSrc[t] );
  return ret;
  }
*/
