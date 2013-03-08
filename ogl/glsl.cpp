#include "glsl.h"
/*
#include <GL/glew.h>
#include <GL/gl.h>

#include <cassert>

#include <Tempest/VertexShader>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>

#include "shading/uniformcash.h"
#include <Tempest/Uniform>

#include <iostream>
#include <fstream>

using namespace Tempest;

struct GLSL::Data{
  AbstractAPI::OpenGL2xDevice * context;

  static void dbgOut( GLuint context ){

    }

  void dbg(){
    //dbg(false, context);
    }

  static void dbg( bool ok, GLuint context ){

    }

  GLuint currentProgramVS, currentProgramFS;
  Detail::UniformCash<GLuint> vsCash, fsCash;

  std::string readFile( const char* f ){
    std::ifstream is( f, std::ifstream::binary );
    assert(is);

    is.seekg (0, is.end);
    int length = is.tellg();
    is.seekg (0, is.beg);

    char * buffer = new char [length];
    is.read (buffer,length);

    assert(is);
    is.close();

    return buffer;
    }

  GLuint loadShader( GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);

    if (shader) {
      glShaderSource(shader, 1, &pSource, NULL);
      glCompileShader(shader);
      GLint compiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

      if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen) {
          char* buf = new char[infoLen];

          if (buf) {
            glGetShaderInfoLog(shader, infoLen, NULL, buf);
            std::cerr << buf;
            delete[] (buf);
            }

          glDeleteShader(shader);
          shader = 0;
          }

        return 0;
        }
      }
    return shader;
    }

  };
//LPDIRECT3DDEVICE9 GLSL::Data::currentDev = 0;

void* GLSL::context() const{
  return data->context;
  }

GLSL::GLSL( AbstractAPI::OpenGL2xDevice * dev ) {
  data = new Data();
  data->currentProgramVS = 0;
  data->currentProgramFS = 0;

  data->context = dev;
  }

GLSL::~GLSL(){
  setNullDevice();
  delete data;
  }

void Tempest::GLSL::beginPaint() const {

  }

void Tempest::GLSL::endPaint() const {
  data->currentProgramVS = 0;
  data->vsCash.reset();
  //cgGLDisableProfile( data->vertexProfile );

  data->currentProgramFS = 0;
  data->fsCash.reset();
  //cgGLDisableProfile( data->pixelProfile );
  }

void GLSL::setDevice() const {

  }

void GLSL::setNullDevice(){

  }

AbstractShadingLang::VertexShader*
    GLSL::createVertexShader( const std::string& fname ) const {
  return createVertexShaderFromSource( data->readFile(fname.data()) );
  }

AbstractShadingLang::VertexShader*
  GLSL::createVertexShaderFromSource(const std::string &src) const {
  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_VERTEX_SHADER, src.data() );

  if( !*prog )
    ;//Data::dbgOut( data->context );

  assert( *prog );

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

void GLSL::deleteVertexShader( VertexShader* s ) const {
  //setNullDevice();
  GLuint* prog = (GLuint*)(s);
  glDeleteShader( *prog );
  }

AbstractShadingLang::FragmentShader*
    GLSL::createFragmentShader( const std::string& fname ) const{
  return createFragmentShaderFromSource( data->readFile(fname.data()) );
  }

AbstractShadingLang::FragmentShader *
  GLSL::createFragmentShaderFromSource(const std::string &src) const {
  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_FRAGMENT_SHADER, src.data() );

  if( !*prog )
    ;//Data::dbgOut( data->context );

  assert( *prog );

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

void GLSL::deleteFragmentShader( FragmentShader* s ) const{
  GLuint* prog = (GLuint*)(s);
  glDeleteShader( *prog );
  }

void GLSL::bind( const Tempest::VertexShader& s ) const {
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramVS!=prog ){
    data->currentProgramVS = prog;
    cgGLBindProgram( prog );
    cgGLEnableProfile( data->vertexProfile );
    }

  //setDevice();
  }

void GLSL::bind( const Tempest::FragmentShader& s ) const {
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramFS!=prog ){
    data->currentProgramFS = prog;
    cgGLBindProgram( prog );
    cgGLEnableProfile( data->pixelProfile );
    }

  //setDevice();
  }

void GLSL::unBind( const Tempest::VertexShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentProgramVS = 0;
  data->vsCash.reset();

  //setDevice();
  }

void GLSL::unBind( const Tempest::FragmentShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentProgramFS = 0;
  data->fsCash.reset();

  //setDevice();
  }

void GLSL::setUniform( Tempest::VertexShader &s,
                        const Uniform<float[2]> &u,
                        Detail::ShInput &  ) const {
  GLuint   prog = GLuint( get(s) );

  GLuint prm = glUniformL ( prog, u.name().data() );

  const float v[4] = { u[0], u[1], 0, 0 };
  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, 2) ){
    Data::dbg(prm, data->context);
    glUniform2fv( prm, v );
    }

  }

void GLSL::setUniform( Tempest::VertexShader &s,
                        const Uniform<float[3]> &u,
                        Detail::ShInput &  ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], u[2], 0 };
  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, 3) ){
    Data::dbg(prm, data->context);
    cgGLSetParameter3fv( prm, v );
    }

  }

void GLSL::setUniform( Tempest::VertexShader &s,
                        const Matrix4x4& mIn,
                        const char *name  ) const{
  //setDevice();
  Matrix4x4 m = mIn;

  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name );

  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );
    cgGLSetMatrixParameterfr( prm, r );
    }
  }

void GLSL::setUniform( Tempest::VertexShader &s,
                        const float v[],
                        int l,
                        const char *name ) const{
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name);

  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, l) ){
    Data::dbg(prm, data->context);

    switch(l){
      case 1: cgGLSetParameter1fv( prm, v ); break;
      case 2: cgGLSetParameter2fv( prm, v ); break;
      case 3: cgGLSetParameter3fv( prm, v ); break;
      case 4: cgGLSetParameter4fv( prm, v ); break;
      }
    }
  }

void GLSL::setUniform( Tempest::FragmentShader &s,
                        const Uniform<float[2]> &u,
                        Detail::ShInput & ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], 0, 0 };
  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, 2) ){
    Data::dbg(prm, data->context);
    cgGLSetParameter2fv( prm, v );
    }

  }

void GLSL::setUniform( Tempest::FragmentShader &s,
                        const Uniform<float[3]> &u,
                        Detail::ShInput & ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], u[2], 0 };
  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, 3) ){
    Data::dbg(prm, data->context);
    cgGLSetParameter3fv( prm, v );
    }

  }

void GLSL::setUniform( Tempest::FragmentShader &s,
                        const float v[],
                        int l,
                        const char *name ) const{
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name);

  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, l) ){
    Data::dbg(prm, data->context);

    switch(l){
      case 1: cgGLSetParameter1fv( prm, v ); break;
      case 2: cgGLSetParameter2fv( prm, v ); break;
      case 3: cgGLSetParameter3fv( prm, v ); break;
      case 4: cgGLSetParameter4fv( prm, v ); break;
      }
    }

  }

void GLSL::setUniform( Tempest::FragmentShader &s,
                        const Matrix4x4& mIn,
                        const char *name ) const {
  Matrix4x4 m = mIn;

  CGprogram   prog = CGprogram( get(s) );
  CGparameter prm = cgGetNamedParameter( prog, name );

  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );

    Data::dbg(prm, data->context);
    cgGLSetMatrixParameterfr( prm, r );
    }
  }

void GLSL::setUniform( Tempest::FragmentShader &sh,
                        const Uniform<Texture2d> &u,
                        Detail::ShInput &in ) const {
  if( !u.value() )
    return;

  CGprogram   prog = CGprogram( get(sh) );
  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const Texture2d::Sampler & s = u.value()->sampler();
  Data::dbg(prm, data->context);

  if( prog != data->currentProgramFS ||
      !data->fsCash.fetch(prm, u.value()->handle()) ){
    GLuint* tx = (GLuint*)get(*u.value());

    cgGLSetTextureParameter   ( prm, *tx);
    cgGLEnableTextureParameter( prm );
    }
  }

void GLSL::setUniform( Tempest::FragmentShader &sh,
                        const Texture2d& u,
                        const char *name ) const{
  CGprogram   prog = CGprogram( get(sh) );
  CGparameter prm = cgGetNamedParameter( prog, name );

  const Texture2d::Sampler & s = u.sampler();
  Data::dbg(prm, data->context);

  if( prog != data->currentProgramFS ||
      !data->fsCash.fetch(prm, u.handle()) ){
    GLuint* tx = (GLuint*)get(u);

    if( tx ){
      cgGLSetTextureParameter   ( prm, *tx);
      cgGLEnableTextureParameter( prm );
      }
    }
  }
*/
