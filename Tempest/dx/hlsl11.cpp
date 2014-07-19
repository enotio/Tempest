#include "HLSL11.h"

#ifndef _MSC_VER
#define __in
#define __out
#define __in_opt
#define __out_opt
#define __in_ecount_opt(X)
#define __out_ecount_opt(X)
#define __in_ecount(X)
#define __out_ecount(X)
#define __in_bcount(X)
#define __out_bcount(X)
#define __in_bcount_opt(X)
#define __out_bcount_opt(X)
#define __in_opt
#define __inout
#define __inout_opt
#define __in_ecount_part_opt(X,Y)
#define __out_ecount_part_opt(X,Y)
#endif
#include <D3D11.h>
#include <D3Dcompiler.h>

#include <Tempest/Device>
#include <Tempest/VertexDeclaration>
#include "utils/sortedvec.h"
#include <tuple>

using namespace Tempest;

struct HLSL11::Data{
  ID3D11Device*        dev;
  ID3D11DeviceContext* immediateContext;

  template<class Sh>
  struct Shader{
    Sh* shader=0;
    ID3DBlob* blob = 0;

    ~Shader(){
      if(shader)
        shader->Release();
      if(blob)
        blob->Release();
      }
    };

  struct Program {
    const Data::Shader<ID3D11VertexShader>* vs   = 0;
    const Data::Shader<ID3D11PixelShader>*  fs   = 0;
    const VertexDeclaration::Declarator*    decl = 0;

    ID3D11InputLayout* dxDecl = 0;

    bool operator == ( const Program& p ) const {
      return vs==p.vs &&
             fs==p.fs &&
             decl==p.decl;
      }
    bool operator < ( const Program& p ) const {
      return std::tie(vs,fs,decl) < std::tie(p.vs,p.fs,p.decl);
      }
    };
  Program prog;
  SortedVec<Program> sh;

  ID3D11InputLayout* createDecl( const VertexDeclaration::Declarator &de,
                                 ID3DBlob* vsBlob ){
    static const DXGI_FORMAT ct[] = {
      DXGI_FORMAT_UNKNOWN,
      DXGI_FORMAT_R32_FLOAT,
      DXGI_FORMAT_R32G32_FLOAT,
      DXGI_FORMAT_R32G32B32_FLOAT,
      DXGI_FORMAT_R32G32B32A32_FLOAT,

      DXGI_FORMAT_B8G8R8A8_UNORM,

      DXGI_FORMAT_R16G16_SINT,
      DXGI_FORMAT_R16G16B16A16_SINT,

      DXGI_FORMAT_R16G16_FLOAT,
      DXGI_FORMAT_R16G16B16A16_FLOAT,
      };

    static const char* usage[] = {
      "POSITION",
      "BLENDWEIGHT",   // 1
      "BLENDINDICES",  // 2
      "NORMAL",        // 3
      "PSIZE",         // 4
      "TEXCOORD",      // 5
      "TANGENT",       // 6
      "BINORMAL",      // 7
      "TESSFACTOR",    // 8
      "POSITIONT",     // 9
      "COLOR",         // 10
      "FOG",           // 11
      "DEPTH",         // 12
      "SAMPLE",        // 13
      ""
      };
    /*
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };*/

    std::vector<D3D11_INPUT_ELEMENT_DESC> decl;
    for( int i=0; i<de.size(); ++i ){
      const VertexDeclaration::Declarator::Element& ex = de[i];
      D3D11_INPUT_ELEMENT_DESC e;
      e.SemanticName         = usage[ex.usage];
      e.SemanticIndex        = ex.index;
      e.Format               = ct[ex.component];
      e.InputSlot            = 0;
      e.AlignedByteOffset    = 4*ex.component;
      e.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
      e.InstanceDataStepRate = 0;

      if( de[i].component==Decl::color )
        e.AlignedByteOffset = 4;

      if( de[i].component==Decl::short2 ||
          de[i].component==Decl::half2 )
        e.AlignedByteOffset = 4;

      if( de[i].component==Decl::short4 ||
          de[i].component==Decl::half4 )
        e.AlignedByteOffset = 8;

      decl.push_back(e);
      }

    for( size_t i=1; i<decl.size(); ++i ){
      decl[i].AlignedByteOffset += decl[i-1].AlignedByteOffset;
      }

    if( decl.size() ){
      for( size_t i=decl.size()-1; i>=1; --i ){
        decl[i].AlignedByteOffset = decl[i-1].AlignedByteOffset;
        }
      decl[0].AlignedByteOffset = 0;
      }

    ID3D11InputLayout* ret = NULL;
    HRESULT hr = dev->CreateInputLayout(
                   decl.data(),
                   decl.size(),
                   vsBlob->GetBufferPointer(),
                   vsBlob->GetBufferSize(),
                   &ret);
    T_ASSERT(SUCCEEDED(hr));
    return ret;
    }

  void setSampler( int i, const Tempest::Texture2d::Sampler& s ){
    /*
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
    */
    }

  void setSampler( int i, const Tempest::Texture3d::Sampler& s ){
    /*
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
    */
    }
  };

HLSL11::HLSL11(AbstractAPI::DirectX11Device *dev , void *context) {
  data = new Data();
  data->dev = (ID3D11Device*)dev;
  data->immediateContext = (ID3D11DeviceContext*)context;
  }

HLSL11::~HLSL11() {
  delete data;
  }

void HLSL11::bind( const Tempest::VertexShader & vs ) const {
  data->prog.vs = (Data::Shader<ID3D11VertexShader>*)get(vs);
  }

void HLSL11::bind( const Tempest::FragmentShader & fs ) const {
  data->prog.fs = (Data::Shader<ID3D11PixelShader>*)get(fs);
  }

void *HLSL11::context() const {
  return data->dev;
  }

void Tempest::HLSL11::setVertexDecl(const AbstractAPI::VertexDecl *d) const {
  data->prog.decl = (VertexDeclaration::Declarator*)d;
  }

GraphicsSubsystem::VertexShader*
  HLSL11::createVertexShaderFromSource( const std::string &src,
                                        std::string &outputLog) const {
  ID3DBlob *errors = 0;
  HRESULT result;

  Data::Shader<ID3D11VertexShader>* sh = new Data::Shader<ID3D11VertexShader>();

  result = D3DCompile( src.data(),
                       src.size(),
                       NULL,            //src-name
                       NULL,            //macro's
                       NULL,            //includes
                       "main",          //main function
                       "vs_4_0",        //shader profile
                       0,               //flags1
                       0,               //flags2
                       &sh->blob,       //compiled operations
                       &errors);        //errors
  if( FAILED(result) ){
    if(errors){
      outputLog.resize( errors->GetBufferSize() );
      memcpy( &outputLog[0], errors->GetBufferPointer(), errors->GetBufferSize() );
      errors->Release();
      }
    delete sh;
    return 0;
    }
  if( errors )
    errors->Release();

  data->dev->CreateVertexShader( sh->blob->GetBufferPointer(),
                                 sh->blob->GetBufferSize(),
                                 NULL,
                                 &sh->shader );

  return (GraphicsSubsystem::VertexShader*)sh;
  }

void HLSL11::deleteShader(GraphicsSubsystem::VertexShader *s) const {
  Data::Shader<ID3D11VertexShader>* vs = (Data::Shader<ID3D11VertexShader>*)s;
  if(data->prog.vs==vs)
    data->prog.vs=0;

  size_t nsz = 0;
  for( size_t i=0; i<data->sh.size(); ++i){
    data[nsz]=data[i];
    if(s==(GraphicsSubsystem::VertexShader*)data->sh[i].vs){
      data->sh[i].dxDecl->Release();
      } else {
      ++nsz;
      }
    }
  data->sh.data.resize(nsz);

  delete vs;
  }

GraphicsSubsystem::FragmentShader*
  HLSL11::createFragmentShaderFromSource( const std::string &src,
                                        std::string &outputLog ) const {
  ID3DBlob* errors = 0;
  HRESULT result;

  Data::Shader<ID3D11PixelShader>* sh = new Data::Shader<ID3D11PixelShader>();

  result = D3DCompile( src.data(),
                       src.size(),
                       NULL,            //src-name
                       NULL,            //macro's
                       NULL,            //includes
                       "main",          //main function
                       "ps_4_0",        //shader profile
                       0,               //flags1
                       0,               //flags2
                       &sh->blob,       //compiled operations
                       &errors);        //errors
  if( FAILED(result) ){
    if(errors){
      outputLog.resize( errors->GetBufferSize() );
      memcpy( &outputLog[0], errors->GetBufferPointer(), errors->GetBufferSize() );
      errors->Release();
      }
    delete sh;
    return 0;
    }

  if( errors )
    errors->Release();

  data->dev->CreatePixelShader( sh->blob->GetBufferPointer(),
                                sh->blob->GetBufferSize(),
                                NULL,
                                &sh->shader );

  return (GraphicsSubsystem::FragmentShader*)sh;
  }

void HLSL11::deleteShader(GraphicsSubsystem::FragmentShader *s) const {
  Data::Shader<ID3D11PixelShader>* fs = (Data::Shader<ID3D11PixelShader>*)s;
  if(data->prog.fs==fs)
    data->prog.fs=0;

  size_t nsz = 0;
  for( size_t i=0; i<data->sh.size(); ++i){
    data[nsz]=data[i];
    if(s==(GraphicsSubsystem::FragmentShader*)data->sh[i].fs){
      data->sh[i].dxDecl->Release();
      } else {
      ++nsz;
      }
    }
  data->sh.data.resize(nsz);

  delete fs;
  }

void HLSL11::enable() const {
  Data::Shader<ID3D11VertexShader>* vs =
      (Data::Shader<ID3D11VertexShader>*)data->prog.vs;
  Data::Shader<ID3D11PixelShader>*  fs =
      (Data::Shader<ID3D11PixelShader>*)data->prog.fs;

  auto l = data->sh.find(data->prog);
  if( l!=data->sh.end() ){
    data->immediateContext->IASetInputLayout(l->dxDecl);
    } else {
    data->prog.dxDecl = data->createDecl(*data->prog.decl, vs->blob);
    data->sh.insert(data->prog);
    data->immediateContext->IASetInputLayout(data->prog.dxDecl);
    }
/*
  setUniforms( fs->uniform, inputOf( *data->fs ), true  );
  setUniforms( vs->uniform, inputOf( *data->vs ), false );
  */

  data->immediateContext->VSSetShader( vs->shader, NULL, 0 );
  data->immediateContext->PSSetShader( fs->shader, NULL, 0 );
  }

template< class Sh>
void HLSL11::setUniforms( Sh* prog,
                        const ShaderInput &in,
                        bool textures ) const {
  /*
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
  */
  }

std::string HLSL11::surfaceShader( GraphicsSubsystem::ShaderType t,
                                 const AbstractShadingLang::UiShaderOpt &opt,
                                 bool &hasHalfpixOffset ) const {
  hasHalfpixOffset = true;

  static const std::string vs_src =
      "struct VS_Input {"
      "  float2 Position:  POSITION;"
      "  float2 TexCoord:  TEXCOORD;"
      "  float4 TexCoord1: TEXCOORD1;"
      "  };"

      "struct FS_Input {"
      "  float4 pos: SV_POSITION;"
      "  float2 tc : TEXCOORD0;"
      "  float4 cl : COLOR;"
      "  };"

      "FS_Input main( VS_Input vs ){"
        "FS_Input fs;"
        "fs.tc  = float2( vs.TexCoord.x, 1.0-vs.TexCoord.y );"
        "fs.cl  = vs.TexCoord1;"
        "fs.pos = float4(vs.Position, 0.0, 1.0);"
        "return fs;"
        "}";

  static const std::string fs_src =
      "struct FS_Input {"
      "  float4 pos: SV_POSITION;"
      "  float2 tc : TEXCOORD0;"
      "  float4 cl : COLOR;"
      "  };"

      "float4 main( FS_Input fs ): SV_Target {"
        "return float4(1.0,1.0,1.0,1.0);"//tex2D(brush_texture, float4(fs.tc, 0.0, 0.0) )*fs.cl;"
        "}";


  static const std::string vs_src_nt =
      "struct VS_Input {"
      "  float2 Position:  POSITION;"
      "  float2 TexCoord:  TEXCOORD;"
      "  float4 TexCoord1: TEXCOORD1;"
      "  };"

      "struct FS_Input {"
      "  float4 pos: SV_POSITION;"
      "  float4 cl : COLOR;"
      "  };"

      "FS_Input main( VS_Input vs ) {"
        "FS_Input fs;"
        "fs.cl  = vs.TexCoord1;"
        "fs.pos = float4(vs.Position, 0.0, 1.0);"
        "return fs;"
        "}";

  static const std::string fs_src_nt =
      "struct FS_Input {"
      "  float4 pos: SV_POSITION;"
      "  float4 cl : COLOR;"
      "  };"

      "float4 main( FS_Input fs ): SV_Target {"
        "return fs.cl;"
        "}";

  if( opt.hasTexture ){
    switch( t ) {
      case Vertex:
        return vs_src;
        break;
      case Fragment:
        return fs_src;
        break;
      default:
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
    default:
      break;
    }

  return fs_src;
  }

void Tempest::HLSL11::event(const GraphicsSubsystem::DeleteEvent &e) {
  if(!e.declaration)
    return;

  size_t nsz = 0;
  for( size_t i=0; i<data->sh.size(); ++i){
    data[nsz]=data[i];
    if(e.declaration==(VertexDecl*)data->sh[i].decl){
      data->sh[i].dxDecl->Release();
      } else {
      ++nsz;
      }
    }
  data->sh.data.resize(nsz);
  }
