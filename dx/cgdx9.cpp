#include "cgdx9.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <Cg/Cg.h>
#include <Cg/CgD3D9.h>

#include <cassert>

#include <Tempest/VertexShader>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>

#include "shading/uniformcash.h"
#include <Tempest/Uniform>

#include <iostream>

using namespace Tempest;

struct CgDx9::Data{
  CGcontext context;
  LPDIRECT3DDEVICE9 device;
  D3DCAPS9 caps;

  CGprofile vertexProfile,
            pixelProfile;

  static LPDIRECT3DDEVICE9 currentDev;

  static void dbgOut( CGcontext context ){
    const char* str = cgGetLastListing( context );
    if( str )
      std::cout << str << std::endl;
    }

  static void dbg( bool ok, CGcontext context ){
    if( ok )
      return;

    const char* str = cgGetLastListing( context );
    if( str )
      std::cout << str << std::endl;

    assert(ok);
    }

  CGprogram currentProgramVS, currentProgramFS;
  Detail::UniformCash<CGparameter> vsCash, fsCash;
  };

LPDIRECT3DDEVICE9 CgDx9::Data::currentDev = 0;

void* CgDx9::context() const{
  return data->context;
  }

CgDx9::CgDx9( AbstractAPI::DirectX9Device *dev ) {
  data = new Data();
  data->currentProgramVS = 0;
  data->currentProgramFS = 0;

  data->context = cgCreateContext();
  cgD3D9SetManageTextureParameters( data->context, CG_TRUE );

  data->device  = LPDIRECT3DDEVICE9(dev);

  setDevice();
  data->vertexProfile = cgD3D9GetLatestVertexProfile();
  data->pixelProfile  = cgD3D9GetLatestPixelProfile();

  data->device->GetDeviceCaps( &data->caps );
  }

CgDx9::~CgDx9(){
  setNullDevice();

  cgDestroyContext( data->context );
  delete data;
  }

void CgDx9::setDevice() const {
  if( CgDx9::Data::currentDev != data->device ){
    cgD3D9SetDevice( data->device );
    CgDx9::Data::currentDev = data->device;
    }

  }

void CgDx9::setNullDevice(){
  if( CgDx9::Data::currentDev != 0 ){
    cgD3D9SetDevice( 0 );
    CgDx9::Data::currentDev = 0;
    }
  }

AbstractShadingLang::VertexShader*
    CgDx9::createVertexShader( const std::string& fname ) const {

  const char **vertexOptions[] =  {
    cgD3D9GetOptimalOptions( data->vertexProfile ),
    0,
    };

  //setDevice();
  CGprogram  prog = cgCreateProgramFromFile( data->context,
                                             CG_SOURCE,
                                             (char*)fname.data(),
                                             data->vertexProfile,
                                             "main",
                                             *vertexOptions );

  if( !prog )
    Data::dbgOut( data->context );

  cgD3D9LoadProgram( prog, false, 0 );

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

AbstractShadingLang::VertexShader*
  CgDx9::createVertexShaderFromSource(const std::string &src) const {
  const char **vertexOptions[] =  {
    cgD3D9GetOptimalOptions( data->vertexProfile ),
    0,
    };

  //setDevice();
  CGprogram prog = cgCreateProgram( data->context,
                                    CG_SOURCE,
                                    src.data(),
                                    data->vertexProfile,
                                    "main",
                                    *vertexOptions );

  if( !prog )
    Data::dbgOut( data->context );

  cgD3D9LoadProgram( prog, false, 0 );

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

void CgDx9::deleteVertexShader( VertexShader* s ) const {
  //setNullDevice();
  cgD3D9UnloadProgram( CGprogram(s) );
  cgDestroyProgram( CGprogram(s) );
  }

AbstractShadingLang::FragmentShader*
    CgDx9::createFragmentShader( const std::string& fname ) const{

  const char **pixelOptions[] = {
    cgD3D9GetOptimalOptions( data->pixelProfile ),
    0,
    };

  //setDevice();
  CGprogram  prog = cgCreateProgramFromFile( data->context,
                                             CG_SOURCE,
                                             (char*)fname.data(),
                                             data->pixelProfile,
                                             "main",
                                             *pixelOptions );

  if( !prog )
    Data::dbgOut( data->context );

  cgD3D9LoadProgram( prog, false, 0 );
  assert( prog );

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

AbstractShadingLang::FragmentShader *
  CgDx9::createFragmentShaderFromSource(const std::string &src) const {
  const char **pixelOptions[] = {
    cgD3D9GetOptimalOptions( data->pixelProfile ),
    0,
    };

  //setDevice();
  CGprogram prog = cgCreateProgram( data->context,
                                    CG_SOURCE,
                                    src.data(),
                                    data->pixelProfile,
                                    "main",
                                    *pixelOptions );

  if( !prog )
    Data::dbgOut( data->context );

  cgD3D9LoadProgram( prog, false, 0 );

  assert( prog );

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

void CgDx9::deleteFragmentShader( FragmentShader* s ) const{
  //setNullDevice();
  cgD3D9UnloadProgram( CGprogram(s) );
  cgDestroyProgram( CGprogram(s) );
  }

void CgDx9::bind( const Tempest::VertexShader& s ) const {
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramVS!=prog ){
    data->currentProgramVS = prog;
    data->vsCash.reset();
    cgD3D9BindProgram( prog );
    }

  //setDevice();
  }

void CgDx9::bind( const Tempest::FragmentShader& s ) const {
  CGprogram prog = CGprogram( get(s) );

  if( data->currentProgramFS!=prog ){
    data->currentProgramFS = prog;
    data->fsCash.reset();
    cgD3D9BindProgram( prog );
    }

  //setDevice();
  }

void CgDx9::unBind( const Tempest::VertexShader& s ) const {
  CGprogram prog = CGprogram( get(s) );
  data->currentProgramVS = 0;

  //setDevice();
  cgD3D9UnbindProgram( prog );
  }

void CgDx9::unBind( const Tempest::FragmentShader& s ) const {
  CGprogram prog = CGprogram( get(s) );
  data->currentProgramFS = 0;

  //setDevice();
  cgD3D9UnbindProgram( prog );
  }

void CgDx9::setUniform( Tempest::VertexShader &s,
                        const Uniform<float[2]> &u,
                        Detail::ShInput & /*in*/ ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], 0, 0 };
  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, 2) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }

  }

void CgDx9::setUniform( Tempest::VertexShader &s,
                        const Uniform<float[3]> &u,
                        Detail::ShInput & /*in*/ ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], u[2], 0 };
  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, 3) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }

  }

void CgDx9::setUniform( Tempest::VertexShader &s,
                     const Matrix4x4& mIn,
                     const char *name  ) const{
  //setDevice();
  Matrix4x4 m = mIn;
  m.transpose();

  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name );

  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );

    D3DXMATRIX matr = D3DXMATRIX(r);

    Data::dbg(prm, data->context);
    cgD3D9SetUniformMatrix( prm, &matr );
    }
  }

void CgDx9::setUniform(Tempest::VertexShader &s,
                     const float v[],
                     int l, const char *name ) const{
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name);

  if( prog != data->currentProgramVS || !data->vsCash.fetch(prm, v, l) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }
  }

void CgDx9::setUniform( Tempest::FragmentShader &s,
                        const Uniform<float[2]> &u,
                        Detail::ShInput & /*in*/ ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], 0, 0 };
  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, 2) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }

  }

void CgDx9::setUniform( Tempest::FragmentShader &s,
                        const Uniform<float[3]> &u,
                        Detail::ShInput & /*in*/ ) const {
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const float v[4] = { u[0], u[1], u[2], 0 };
  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, 3) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }

  }

void CgDx9::setUniform( Tempest::FragmentShader &sh,
                     const Uniform<Texture2d> &u,
                     Detail::ShInput &in) const {
  if( !u.value() )
    return;

  CGprogram   prog = CGprogram( get(sh) );
  CGparameter prm = cgGetNamedParameter( prog, u.name().data() );

  const Texture2d::Sampler & s = u.value()->sampler();
  Data::dbg(prm, data->context);

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

  if( prog != data->currentProgramFS ||
      !data->fsCash.fetch(prm, u.value()->handle()) ){
    cgD3D9SetTexture( prm,
                      LPDIRECT3DTEXTURE9( get(*u.value()) ) );
    }
  }

void CgDx9::setUniform(Tempest::FragmentShader &s,
                     const float v[],
                     int l,
                     const char *name ) const{
  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name);

  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, v, l) ){
    Data::dbg(prm, data->context);
    cgD3D9SetUniform( prm, v );
    }

  }

void CgDx9::setUniform( Tempest::FragmentShader &s,
                     const Matrix4x4& mIn,
                     const char *name ) const {
  Matrix4x4 m = mIn;
  m.transpose();

  CGprogram   prog = CGprogram( get(s) );

  CGparameter prm = cgGetNamedParameter( prog, name );

  if( prog != data->currentProgramFS || !data->fsCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );

    D3DXMATRIX matr = D3DXMATRIX(r);

    Data::dbg(prm, data->context);
    cgD3D9SetUniformMatrix( prm, &matr );
    }
  }

void CgDx9::setUniform( Tempest::FragmentShader &sh,
                     const Texture2d& texture,
                     const char *name ) const{
  CGprogram   prog = CGprogram( get(sh) );
  CGparameter prm = cgGetNamedParameter( prog, name );

  const Texture2d::Sampler & s = texture.sampler();
  Data::dbg(prm, data->context);

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

  if( prog != data->currentProgramFS ||
      !data->fsCash.fetch(prm, texture.handle()) ){
    cgD3D9SetTexture( prm,
                      LPDIRECT3DTEXTURE9( get(texture) ) );
    }
  }

