#include "CgOGL.h"

#include <GL/gl.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>

#include <cassert>

#include <Tempest/VertexShader>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>

#include "shading/uniformcash.h"
#include <Tempest/Uniform>

#include <iostream>

using namespace Tempest;

struct CgOGL::Data{
  CGcontext context;
  //LPDIRECT3DDEVICE9 device;
  //D3DCAPS9 caps;

  CGprofile vertexProfile,
            pixelProfile;

  //static LPDIRECT3DDEVICE9 currentDev;

  static void dbgOut( CGcontext context ){
    const char* str = cgGetLastListing( context );
    if( str )
      std::cout << str << std::endl;
    }

  void dbg(){
    dbg(false, context);
    }

  static void dbg( bool ok, CGcontext context ){
    if( ok )
      return;

    CGerror error;
    const char *string = cgGetLastErrorString(&error);

    if (error != CG_NO_ERROR) {
      std::cout << string << std::endl;
      if( error == CG_COMPILER_ERROR ){
        const char* str = cgGetLastListing( context );
        std::cout << str << std::endl;
        }

      assert(ok);
      }
    }

  const Tempest::VertexShader*   currentVS;
  const Tempest::FragmentShader* currentFS;

  CGprogram currentProgramVS, currentProgramFS;
  Detail::UniformCash<CGparameter> vsCash, fsCash;
  };

//LPDIRECT3DDEVICE9 CgOGL::Data::currentDev = 0;

void* CgOGL::context() const{
  return data->context;
  }

CgOGL::CgOGL( AbstractAPI::OpenGL2xDevice * ) {
  data = new Data();
  data->currentProgramVS = 0;
  data->currentProgramFS = 0;

  data->currentVS = 0;
  data->currentFS = 0;

  data->context = cgCreateContext();
  data->dbg();

  cgGLSetDebugMode( CG_TRUE );
  cgSetParameterSettingMode(data->context, CG_DEFERRED_PARAMETER_SETTING);

  data->vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  cgGLSetOptimalOptions( data->vertexProfile );

  data->pixelProfile  = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions( data->pixelProfile );
  }

CgOGL::~CgOGL(){
  cgDestroyContext( data->context );
  delete data;
  }

void Tempest::CgOGL::beginPaint() const {

  }

void Tempest::CgOGL::endPaint() const {
  data->currentProgramVS = 0;
  data->vsCash.reset();
  cgGLDisableProfile( data->vertexProfile );

  data->currentProgramFS = 0;
  data->fsCash.reset();
  cgGLDisableProfile( data->pixelProfile );
  }

void Tempest::CgOGL::enable() const {
  { const ShaderInput & in = inputOf( *data->currentFS );

    for( size_t i=0; i<in.tex.names.size(); ++i ){
      setUniform( *data->currentFS, *in.tex.values[i], in.tex.names[i].data() );
      }

    for( size_t i=0; i<in.mat.names.size(); ++i ){
      setUniform( *data->currentFS, in.mat.values[i], in.mat.names[i].data() );
      }

    setUniforms( *data->currentFS, in.v1, 1 );
    setUniforms( *data->currentFS, in.v2, 2 );
    setUniforms( *data->currentFS, in.v3, 3 );
    setUniforms( *data->currentFS, in.v4, 4 );
    }

  { const ShaderInput & in = inputOf( *data->currentVS );

    for( size_t i=0; i<in.mat.names.size(); ++i ){
      setUniform( *data->currentVS, in.mat.values[i], in.mat.names[i].data() );
      }

    setUniforms( *data->currentVS, in.v1, 1 );
    setUniforms( *data->currentVS, in.v2, 2 );
    setUniforms( *data->currentVS, in.v3, 3 );
    setUniforms( *data->currentVS, in.v4, 4 );
    }
  }

template< class Sh, class T >
void CgOGL::setUniforms( const Sh & s, const T & vN, int c ) const{
  for( size_t i=0; i<vN.names.size(); ++i ){
    setUniform( s, vN.values[i].v, c, vN.names[i].data() );
    }
  }

void CgOGL::setDevice() const {
  /*
  if( CgOGL::Data::currentDev != data->device ){
    cgD3D9SetDevice( data->device );
    CgOGL::Data::currentDev = data->device;
    }
  */
  }

AbstractShadingLang::VertexShader*
    CgOGL::createVertexShader( const std::string& fname ) const {
  CGprogram  prog = cgCreateProgramFromFile( data->context,
                                             CG_SOURCE,
                                             (char*)fname.data(),
                                             data->vertexProfile,
                                             "main",
                                             0 );

  if( !prog )
    Data::dbgOut( data->context );

  cgGLLoadProgram(prog);

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

AbstractShadingLang::VertexShader*
  CgOGL::createVertexShaderFromSource(const std::string &src) const {
  CGprogram prog = cgCreateProgram( data->context,
                                    CG_SOURCE,
                                    src.data(),
                                    data->vertexProfile,
                                    "main",
                                    0 );

  if( !prog )
    Data::dbgOut( data->context );

  cgGLLoadProgram(prog);

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

void CgOGL::deleteVertexShader( VertexShader* s ) const {
  //setNullDevice();
  cgGLUnloadProgram( CGprogram(s) );
  cgDestroyProgram( CGprogram(s) );
  }

AbstractShadingLang::FragmentShader*
    CgOGL::createFragmentShader( const std::string& fname ) const{
  //setDevice();
  CGprogram  prog = cgCreateProgramFromFile( data->context,
                                             CG_SOURCE,
                                             (char*)fname.data(),
                                             data->pixelProfile,
                                             "main",
                                             0 );

  if( !prog )
    Data::dbgOut( data->context );

  cgGLLoadProgram( prog );
  assert( prog );

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

AbstractShadingLang::FragmentShader *
  CgOGL::createFragmentShaderFromSource(const std::string &src) const {
  CGprogram prog = cgCreateProgram( data->context,
                                    CG_SOURCE,
                                    src.data(),
                                    data->pixelProfile,
                                    "main",
                                    0 );

  if( !prog )
    Data::dbgOut( data->context );

  cgGLLoadProgram( prog );

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

void CgOGL::deleteFragmentShader( FragmentShader* s ) const{
  //setNullDevice();
  cgGLUnloadProgram( CGprogram(s) );
  cgDestroyProgram( CGprogram(s) );
  }

void CgOGL::bind( const Tempest::VertexShader& s ) const {
  data->currentVS = &s;
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramVS!=prog ){
    data->currentProgramVS = prog;
    cgGLBindProgram( prog );
    cgGLEnableProfile( data->vertexProfile );
    }

  //setDevice();
  }

void CgOGL::bind( const Tempest::FragmentShader& s ) const {
  data->currentFS = &s;
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramFS!=prog ){
    data->currentProgramFS = prog;
    cgGLBindProgram( prog );
    cgGLEnableProfile( data->pixelProfile );
    }

  //setDevice();
  }

void CgOGL::unBind( const Tempest::VertexShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentProgramVS = 0;
  data->vsCash.reset();

  //setDevice();
  cgGLDisableProfile( data->vertexProfile );
  }

void CgOGL::unBind( const Tempest::FragmentShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentProgramFS = 0;
  data->fsCash.reset();

  //setDevice();
  cgGLDisableProfile( data->pixelProfile );
  }

void CgOGL::setUniform( const Tempest::VertexShader &s,
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

void CgOGL::setUniform( const Tempest::VertexShader &s,
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

void CgOGL::setUniform( const Tempest::FragmentShader &s,
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

void CgOGL::setUniform( const Tempest::FragmentShader &s,
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

void CgOGL::setUniform( const Tempest::FragmentShader &sh,
                        const Texture2d& u,
                        const char *name ) const{
  CGprogram   prog = CGprogram( get(sh) );
  CGparameter prm = cgGetNamedParameter( prog, name );

  const Texture2d::Sampler & s = u.sampler();
  Data::dbg(prm, data->context);

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

  if( prog != data->currentProgramFS ||
      !data->fsCash.fetch(prm, u.handle()) ){
    GLuint* tx = (GLuint*)get(u);

    if( tx ){
      cgGLSetTextureParameter   ( prm, *tx);
      cgGLEnableTextureParameter( prm );
      }
    }
}
