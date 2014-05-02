#include "hlsl.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <Tempest/Device>

using namespace Tempest;

struct HLSL::Data{
  LPDIRECT3DDEVICE9   dev;
  D3DCAPS9            caps;

  const Tempest::VertexShader   * vs;
  const Tempest::FragmentShader * fs;

  template< class Sh >
  struct Shader{
    Sh * shader;
    LPD3DXCONSTANTTABLE uniform;
    };

  void setSampler( int i, const Tempest::Texture2d::Sampler& s ){
    static const D3DTEXTUREADDRESS addR[] = {
      D3DTADDRESS_CLAMP,
      D3DTADDRESS_BORDER,
      D3DTADDRESS_MIRRORONCE, //??
      D3DTADDRESS_MIRROR,
      D3DTADDRESS_WRAP,

      D3DTADDRESS_WRAP
      };

    static const D3DTEXTUREFILTERTYPE filters[] = {
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT
      };

    dev->SetSamplerState(i, D3DSAMP_ADDRESSU, addR[ s.uClamp ] );
    dev->SetSamplerState(i, D3DSAMP_ADDRESSV, addR[ s.vClamp ] );

    if( s.anisotropic ){
      dev->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
      dev->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
      } else {
      dev->SetSamplerState( i, D3DSAMP_MAGFILTER, filters[ s.magFilter ] );
      dev->SetSamplerState( i, D3DSAMP_MINFILTER, filters[ s.minFilter ] );
      }
    dev->SetSamplerState(i, D3DSAMP_MIPFILTER, filters[ s.minFilter ] );

    dev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, caps.MaxAnisotropy );
    }

  void setSampler( int i, const Tempest::Texture3d::Sampler& s ){
    static const D3DTEXTUREADDRESS addR[] = {
      D3DTADDRESS_CLAMP,
      D3DTADDRESS_BORDER,
      D3DTADDRESS_MIRRORONCE, //??
      D3DTADDRESS_MIRROR,
      D3DTADDRESS_WRAP,

      D3DTADDRESS_WRAP
      };

    static const D3DTEXTUREFILTERTYPE filters[] = {
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT
      };

    dev->SetSamplerState(i, D3DSAMP_ADDRESSU, addR[ s.uClamp ] );
    dev->SetSamplerState(i, D3DSAMP_ADDRESSV, addR[ s.vClamp ] );
    dev->SetSamplerState(i, D3DSAMP_ADDRESSW, addR[ s.wClamp ] );

    dev->SetSamplerState( i, D3DSAMP_MAGFILTER, filters[ s.magFilter ] );
    dev->SetSamplerState( i, D3DSAMP_MINFILTER, filters[ s.minFilter ] );
    dev->SetSamplerState( i, D3DSAMP_MIPFILTER, filters[ s.minFilter ] );

    dev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, caps.MaxAnisotropy );
    }
  };

HLSL::HLSL( AbstractAPI::DirectX9Device *dev ) {
  data = new Data();
  data->dev = (LPDIRECT3DDEVICE9)dev;
  data->dev->GetDeviceCaps( &data->caps );
  }

HLSL::~HLSL() {
  delete data;
  }

void HLSL::bind( const Tempest::VertexShader & vs ) const {
  data->vs = &vs;
  }

void HLSL::bind( const Tempest::FragmentShader & fs ) const {
  data->fs = &fs;
  }

void HLSL::unBind(const Tempest::VertexShader &) const {
  data->vs = 0;
  }

void HLSL::unBind(const Tempest::FragmentShader &) const {
  data->fs = 0;
  }

void *HLSL::context() const {
  return data->dev;
  }

GraphicsSubsystem::VertexShader*
  HLSL::createVertexShaderFromSource( const std::string &src,
                                      std::string &outputLog) const {
  LPD3DXBUFFER code = 0, errors = 0;
  HRESULT result;

  Data::Shader<IDirect3DVertexShader9>*
      sh = new Data::Shader<IDirect3DVertexShader9>();

  result = D3DXCompileShader( src.data(),
                              src.size(),
                              NULL,            //macro's
                              NULL,            //includes
                              "main",       //main function
                              "vs_3_0",        //shader profile
                              0,               //flags
                              &code,           //compiled operations
                              &errors,            //errors
                              &sh->uniform ); //constants
  if( FAILED(result) ){
    outputLog.resize( errors->GetBufferSize() );
    memcpy( &outputLog[0], errors->GetBufferPointer(), errors->GetBufferSize() );
    return 0;
    }

  data->dev->CreateVertexShader( (DWORD*)code->GetBufferPointer(),
                                 &sh->shader );

  return (GraphicsSubsystem::VertexShader*)sh;
  }

void HLSL::deleteVertexShader(GraphicsSubsystem::VertexShader *s) const {
  Data::Shader<IDirect3DVertexShader9>* vs = (Data::Shader<IDirect3DVertexShader9>*)s;

  vs->shader->Release();
  vs->uniform->Release();

  delete vs;
  }

GraphicsSubsystem::FragmentShader*
  HLSL::createFragmentShaderFromSource( const std::string &src,
                                        std::string &outputLog ) const {
  LPD3DXBUFFER code = 0, errors = 0;
  HRESULT result;

  Data::Shader<IDirect3DPixelShader9>*
      sh = new Data::Shader<IDirect3DPixelShader9>();
  result = D3DXCompileShader( src.data(),
                              src.size(),
                              NULL,            //macro's
                              NULL,            //includes
                              "main",       //main function
                              "ps_3_0",        //shader profile
                              0,               //flags
                              &code,           //compiled operations
                              &errors,            //errors
                              &sh->uniform ); //constants
  if( FAILED(result) ){
    outputLog.resize( errors->GetBufferSize() );
    memcpy( &outputLog[0], errors->GetBufferPointer(), errors->GetBufferSize() );
    return 0;
    }

  data->dev->CreatePixelShader( (DWORD*)code->GetBufferPointer(),
                                 &sh->shader );

  return (GraphicsSubsystem::FragmentShader*)sh;
  }

void HLSL::deleteFragmentShader(GraphicsSubsystem::FragmentShader *s) const {
  Data::Shader<IDirect3DPixelShader9>* fs = (Data::Shader<IDirect3DPixelShader9>*)s;

  fs->shader->Release();
  fs->uniform->Release();

  delete fs;
  }

void HLSL::enable() const {
  Data::Shader<IDirect3DVertexShader9>* vs =
      (Data::Shader<IDirect3DVertexShader9>*)get(*data->vs);
  Data::Shader<IDirect3DPixelShader9>*  fs =
      (Data::Shader<IDirect3DPixelShader9>*)get(*data->fs);

  setUniforms( fs->uniform, inputOf( *data->fs ), true  );
  setUniforms( vs->uniform, inputOf( *data->vs ), false );

  data->dev->SetVertexShader( vs->shader );
  data->dev->SetPixelShader ( fs->shader );
  }

template< class Sh>
void HLSL::setUniforms( Sh* prog,
                        const ShaderInput &in,
                        bool textures ) const {
  if( textures ){
    int texSlot = 0;
    for( size_t i=0; i<in.tex3d.names.size(); ++i ){
      prog->SetInt( data->dev, in.tex3d.names[i].data(), in.tex3d.id[i] );
      }

    for( size_t i=0; i<in.tex.names.size(); ++i ){
      IDirect3DTexture9* t = (IDirect3DTexture9*)get(*in.tex.values[i]);

      data->setSampler(texSlot, in.tex.values[i]->sampler() );

      data->dev->SetTexture( texSlot, t );

      prog->SetInt( data->dev, in.tex.names[i].data(), texSlot );
      ++texSlot;
      }
    }

  for( size_t i=0; i<in.v1.names.size(); ++i ){
    prog->SetFloat( data->dev, in.v1.names[i].data(), in.v1.id[i] );
    }

  for( size_t i=0; i<in.v2.names.size(); ++i ){
    ShaderInput::Vec<2> vx = in.v2.values[i];
    D3DXVECTOR4 v = {vx.v[0], vx.v[1], 0, 0};
    prog->SetVector( data->dev, in.v2.names[i].data(), &v );
    }

  for( size_t i=0; i<in.v3.names.size(); ++i ){
    ShaderInput::Vec<3> vx = in.v3.values[i];
    D3DXVECTOR4 v = {vx.v[0], vx.v[1], vx.v[2], 0};
    prog->SetVector( data->dev, in.v3.names[i].data(), &v );
    }

  for( size_t i=0; i<in.v4.names.size(); ++i ){
    ShaderInput::Vec<4> vx = in.v4.values[i];
    D3DXVECTOR4 v = { vx.v[0], vx.v[1], vx.v[2], vx.v[3] };
    prog->SetVector( data->dev, in.v4.names[i].data(), &v );
    }

  for( size_t i=0; i<in.mat.names.size(); ++i ){
    const Matrix4x4& m = in.mat.values[i];
    D3DXMATRIX v;
    memcpy(&v, m.data(), 16*sizeof(float));

    prog->SetMatrix( data->dev, in.mat.names[i].data(), &v );
    }
  }

std::string HLSL::surfaceShader( GraphicsSubsystem::ShaderType t,
                                 const AbstractShadingLang::UiShaderOpt &opt,
                                 bool &hasHalfpixOffset ) const {
  hasHalfpixOffset = true;

  static const std::string vs_src =
      "struct VS_Input {"
      "  float2 Position:  POSITION;"
      "  float2 TexCoord:  TEXCOORD0;"
      "  float4 TexCoord1: TEXCOORD1;"
      "  };"

      "struct FS_Input {"
      "  float4 pos: POSITION;"
      "  float2 tc : TEXCOORD0;"
      "  float4 cl : COLOR;"
      "  };"

      "uniform float2 dpos;"

      "FS_Input main( VS_Input vs ){"
        "FS_Input fs;"
        "fs.tc  = float2( vs.TexCoord.x, 1.0-vs.TexCoord.y );"
        "fs.cl  = vs.TexCoord1;"
        "fs.pos = float4(vs.Position+dpos, 0.0, 1.0);"
        "return fs;"
        "}";

  static const std::string fs_src =
      "struct FS_Input {"
      "  float4 pos: POSITION;"
      "  float2 tc : TEXCOORD0;"
      "  float4 cl : COLOR;"
      "  };"

      "struct FS_Output {"
      "  float4 color: COLOR0;"
      "  };"

      "sampler2DRect texture;"

      "FS_Output main( FS_Input fs ) {"
        "FS_Output c;"
        "c.color = texRECTlod(texture, float4(fs.tc, 0.0, 0.0) )*fs.cl;"
        "return c;"
        "}";


  static const std::string vs_src_nt =
      "struct VS_Input {"
      "  float2 Position:  POSITION;"
      "  float2 TexCoord:  TEXCOORD0;"
      "  float4 TexCoord1: TEXCOORD1;"
      "  };"

      "struct FS_Input {"
      "  float4 pos: POSITION;"
      "  float4 cl : COLOR;"
      "  };"

      "uniform float2 dpos;"

      "FS_Input main( VS_Input vs ) {"
        "FS_Input fs;"
        "fs.cl  = vs.TexCoord1;"
        "fs.pos = float4(vs.Position+dpos, 0.0, 1.0);"
        "return fs;"
        "}";

  static const std::string fs_src_nt =
      "struct FS_Input {"
      "  float4 pos: POSITION;"
      "  float4 cl : COLOR;"
      "  };"

      "struct FS_Output {"
      "  float4 color: COLOR0;"
      "  };"

      "FS_Output main( FS_Input fs ) {"
        "FS_Output c;"
        "c.color = fs.cl;"
        "return c;"
        "}";

  if( opt.hasTexture ){
    switch( t ) {
      case Vertex:
        return vs_src;
        break;
      case Fragment:
        return fs_src;
        break;
      }
    }

  switch( t ) {
    case Vertex:
      return vs_src_nt;
      break;
    case Fragment:
      return fs_src_nt;
      break;
    }

  return fs_src;
  }
