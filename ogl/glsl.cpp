#include "glsl.h"


#ifdef __ANDROID__
#include <GLES2/gl2.h>
// #include <SDL.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <cassert>

#include <Tempest/VertexShader>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>

#include "shading/uniformcash.h"
#include <Tempest/Uniform>
#include <Tempest/AbstractSystemAPI>

#include <iostream>
#include <fstream>

#include <cstring>

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

  const Tempest::VertexShader*   currentVS;
  const Tempest::FragmentShader* currentFS;

  Detail::UniformCash<GLuint> vsCash, fsCash;
  const AbstractAPI::VertexDecl* vdecl;

  std::string readFile( const char* f ){
    return AbstractSystemAPI::loadText(f).data();
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

  GLint location( GLuint prog, const char* name ){
    if( !cgcgen )
      return glGetUniformLocation( prog, name );

    static std::vector<char> n;
    size_t sz = strlen(name)+1;

    if( n.size() <= sz )
      n.resize( sz+2 );

    n[0] = '_';
    memcpy( &n[1], name, sz );
    n[sz  ] = '1';
    n[sz+1] = '\0';

    // std::cout << n.data() << std::endl;

    return glGetUniformLocation( prog, &n[0] );
    }

  struct ShProgram{
    const Tempest::VertexShader*   vs;
    const Tempest::FragmentShader* fs;

    GLuint linked;
    };

  std::vector<ShProgram> prog;
  bool cgcgen;
  };

void* GLSL::context() const{
  return data->context;
  }

GLSL::GLSL( AbstractAPI::OpenGL2xDevice * dev ) {
  data = new Data();
  data->context = dev;

  data->currentVS = 0;
  data->currentFS = 0;

  data->context = dev;
  data->cgcgen  = false;
  }

GLSL::~GLSL(){
  delete data;
  }

void Tempest::GLSL::beginPaint() const {

  }

void Tempest::GLSL::endPaint() const {
  glUseProgram( 0 );

  data->currentVS = 0;
  data->vsCash.reset();
  //cgGLDisableProfile( data->vertexProfile );

  data->currentFS = 0;
  data->fsCash.reset();
  //cgGLDisableProfile( data->pixelProfile );
  }

void GLSL::setDevice() const {

  }

AbstractShadingLang::VertexShader*
    GLSL::createVertexShader( const std::string& fname ) const {
  std::string src = data->readFile(fname.data());
  return createVertexShaderFromSource( src );
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

  for( size_t i=0; i<data->prog.size();  ){
    if( get( *data->prog[i].vs ) == s ){
      glDeleteProgram( data->prog[i].linked );
      data->prog[i] = data->prog.back();
      data->prog.pop_back();
      } else
      ++i;
    }
  }

AbstractShadingLang::FragmentShader*
    GLSL::createFragmentShader( const std::string& fname ) const{
  std::string src = data->readFile(fname.data());
  return createFragmentShaderFromSource( src );
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

  for( size_t i=0; i<data->prog.size();  ){
    if( get( *data->prog[i].fs ) == s ){
      glDeleteProgram( data->prog[i].linked );
      data->prog[i] = data->prog.back();
      data->prog.pop_back();
      } else
      ++i;
    }
  }

void GLSL::bind( const Tempest::VertexShader& s ) const {
  if( data->currentVS == &s )
    return;

  data->currentVS = &s;
  data->vsCash.reset();
  }

void GLSL::bind( const Tempest::FragmentShader& s ) const {
  if( data->currentFS == &s )
    return;

  data->currentFS = &s;
  data->fsCash.reset();
  }

void GLSL::unBind( const Tempest::VertexShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentVS = 0;
  data->vsCash.reset();

  //setDevice();
  }

void GLSL::unBind( const Tempest::FragmentShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentFS = 0;
  data->fsCash.reset();

  //setDevice();
  }

void GLSL::setVertexDecl(const AbstractAPI::VertexDecl *v ) const {
  data->vdecl = v;
  }

void GLSL::enable() const {
  GLuint vertexShader = *(GLuint*)get( *data->currentVS );
  GLuint pixelShader  = *(GLuint*)get( *data->currentFS );
  GLuint program = 0;

  for( size_t i=0; i<data->prog.size(); ++i )
    if( data->prog[i].vs==data->currentVS &&
        data->prog[i].fs==data->currentFS ){
      program = data->prog[i].linked;
      }

  if( program==0 ){
    program = glCreateProgram();
    assert( program );

    glAttachShader(program, vertexShader);
    glAttachShader(program, pixelShader);

    static const char* uType[] = {
      "Position",
      "BlendWeight",   // 1
      "BlendIndices",  // 2
      "Normal",        // 3
      "PSize",         // 4
      "TexCoord",      // 5
      "Tangent",       // 6
      "BiNormal",      // 7
      "TessFactor",    // 8
      "PositionT",     // 9
      "Color",         // 10
      "Fog",           // 11
      "Depth",         // 12
      "Sample",        // 13
      ""
      };

    const Tempest::VertexDeclaration::Declarator& vd
        = *(const Tempest::VertexDeclaration::Declarator*)data->vdecl;
    for( int i=0; i<vd.size(); ++i ){
      glBindAttribLocation( program, i, uType[vd[i].usage] );
      }

    //glBindAttribLocation( program, 0, "vPosition" );
    glLinkProgram (program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
      GLint bufLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);

      if (bufLength) {
        char* buf = new char[bufLength];
        if (buf) {
          glGetProgramInfoLog(program, bufLength, NULL, buf);
          std::cerr << buf;
          delete[] (buf);
          assert(0);
          }
        }

      glDeleteProgram(program);
      program = 0;
      }

    Data::ShProgram s;
    s.vs = data->currentVS;
    s.fs = data->currentFS;
    s.linked = program;
    data->prog.push_back( s );

    /*
    GLint vl = glGetAttribLocation ( program, "Position");
    GLint cl = glGetAttribLocation ( program, "COLOR");
    std::cout << vl;*/
    }

  glUseProgram( program );
  { const ShaderInput & in = inputOf( *data->currentFS );

    for( size_t i=0; i<in.tex.names.size(); ++i ){
      setUniform( program, *in.tex.values[i], in.tex.names[i].data(), i );
      }

    for( size_t i=0; i<in.mat.names.size(); ++i ){
      setUniform( program, in.mat.values[i], in.mat.names[i].data() );
      }

    setUniforms( program, in.v1, 1 );
    setUniforms( program, in.v2, 2 );
    setUniforms( program, in.v3, 3 );
    setUniforms( program, in.v4, 4 );
    }

  { const ShaderInput & in = inputOf( *data->currentVS );

    for( size_t i=0; i<in.mat.names.size(); ++i ){
      setUniform( program, in.mat.values[i], in.mat.names[i].data() );
      }

    setUniforms( program, in.v1, 1 );
    setUniforms( program, in.v2, 2 );
    setUniforms( program, in.v3, 3 );
    setUniforms( program, in.v4, 4 );
    }

  //glUseProgram( 0 ); //FIXME!!!
  }

template< class T >
void GLSL::setUniforms( unsigned int s, const T & vN, int c ) const{
  for( size_t i=0; i<vN.names.size(); ++i ){
    setUniform( s, vN.values[i].v, c, vN.names[i].data() );
    }
  }

void GLSL::setUniform( unsigned int s,
                       const Matrix4x4& mIn,
                       const char *name  ) const{
  //setDevice();
  Matrix4x4 m = mIn;

  GLuint   prog = s;
  GLint    prm = data->location( prog, name );

  if( !data->vsCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );
    glUniformMatrix4fv( prm, 1, false, r );
    //glUniform4fv( prm, 4, r );
    }
  }

void GLSL::setUniform( unsigned int s,
                       const float v[],
                       int l,
                       const char *name ) const{
  GLuint    prog = s;
  GLint     prm = data->location( prog, name );

  if( !data->vsCash.fetch(prm, v, l) ){
    //Data::dbg(prm, data->context);

    switch(l){
      case 1: glUniform1fv( prm, 1, v ); break;
      case 2: glUniform2fv( prm, 1, v ); break;
      case 3: glUniform3fv( prm, 1, v ); break;
      case 4: glUniform4fv( prm, 1, v ); break;
      }
    }
  }

void GLSL::setUniform( unsigned int sh,
                       const Texture2d& u,
                       const char *name,
                       int slot ) const{
  GLuint    prog = sh;
  GLint     prm = data->location( prog, name );

  const Texture2d::Sampler & s = u.sampler();
  //Data::dbg(prm, data->context);

  /*
  D3DTEXTUREADDRESS addR[] = {
    D3DTADDRESS_CLAMP,
    D3DTADDRESS_BORDER,
    D3DTADDRESS_MIRRORONCE, //??
    D3DTADDRESS_MIRROR,
    D3DTADDRESS_WRAP,

    D3DTADDRESS_WRAP
    };

  cgD3D9SetSamplerState( prm,
                         D3DSAMP_ADDRESSU,
                         addR[ s.uClamp ]);
  cgD3D9SetSamplerState( prm,
                         D3DSAMP_ADDRESSV,
                         addR[ s.vClamp ] );

  D3DTEXTUREFILTERTYPE filters[] = {
    D3DTEXF_POINT,
    D3DTEXF_LINEAR,
    D3DTEXF_POINT
    };

  if( s.anisotropic ){
    cgD3D9SetSamplerState( prm,
                           D3DSAMP_MINFILTER,
                           D3DTEXF_ANISOTROPIC );
    cgD3D9SetSamplerState( prm,
                           D3DSAMP_MAGFILTER,
                           D3DTEXF_LINEAR );
    } else {
    cgD3D9SetSamplerState( prm,
                           D3DSAMP_MINFILTER,
                           filters[ s.minFilter ] );
    cgD3D9SetSamplerState( prm,
                           D3DSAMP_MAGFILTER,
                           filters[ s.magFilter ]);
    }
  cgD3D9SetSamplerState(prm, D3DSAMP_MAXANISOTROPY, data->caps.MaxAnisotropy );

  cgD3D9SetSamplerState( prm,
                         D3DSAMP_MIPFILTER,
                         D3DTEXF_LINEAR );
                         //filters[ s.mipFilter ]);//D3DTEXF_NONE

  cgD3D9SetTextureWrapMode(prm, 0);
  */

  if( !data->fsCash.fetch(prm, u.handle()) ){
    GLuint* tx = (GLuint*)get(u);

    if( tx ){
      //cgGLSetTextureParameter   ( prm, *tx);
      //cgGLEnableTextureParameter( prm );
      glActiveTexture( GL_TEXTURE0 + slot );
      glBindTexture( GL_TEXTURE_2D, *tx );
      glUniform1i( prm, slot );

      glActiveTexture( GL_TEXTURE0 );
      }
    }
}
