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

#include "dx11types.h"
#include <D3D11.h>
#include <D3Dcompiler.h>

#include <Tempest/Device>
#include <Tempest/VertexDeclaration>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>
#include "utils/sortedvec.h"
#include <tuple>
#include <unordered_map>

using namespace Tempest;

struct SmpHash{
  size_t operator()(const Texture2d::Sampler& s) const {
    return (size_t(s.uClamp)*2u+size_t(s.minFilter))*8u + size_t(s.anisotropic?1:0);
    }

  size_t operator()(const Texture3d::Sampler& s) const {
    return (size_t(s.uClamp)*2u+size_t(s.minFilter))*8u;
    }
  };

struct SmpCmp{
  bool operator()(const Texture2d::Sampler& s1,
                    const Texture2d::Sampler& s2) const {
    return s1.magFilter==s2.magFilter &&
           s1.minFilter==s2.minFilter &&
           s1.mipFilter==s2.mipFilter &&
           s1.uClamp==s2.uClamp &&
           s1.vClamp==s2.vClamp;
    }
  };

struct HLSL11::Data{
  ID3D11Device*        dev;
  ID3D11DeviceContext* immediateContext;

  ~Data(){
    for( auto i:samplers2d )
      i.second->Release();
    }

  struct Shader{
    ID3D11VertexShader* vs     = 0;
    ID3D11PixelShader*  fs     = 0;
    ID3DBlob*           blobVs = 0;

    std::shared_ptr<std::vector<ID3D11Buffer*>> ubo;

    ~Shader(){
      if(vs)
        vs->Release();
      if(blobVs)
        blobVs->Release();
      if(fs)
        fs->Release();

      if(ubo)
        for( ID3D11Buffer* u:*ubo )
          if(u)
            u->Release();
      }
    };

  ID3D11Buffer* createUBO( UINT size ){
    if(size==0)
      return 0;
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc,sizeof(desc));

    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = size;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;

    ID3D11Buffer* ret = 0;
    HRESULT hr = dev->CreateBuffer( &desc, NULL, &ret );
    if( FAILED( hr ) )
      return 0;
    return ret;
    }

  std::unordered_map<Texture2d::Sampler,ID3D11SamplerState*,SmpHash,SmpCmp> samplers2d;
  ID3D11SamplerState* createSampler( const Texture2d::Sampler& s ){
    auto scashe = samplers2d.find(s);
    if(scashe!=samplers2d.end())
      return scashe->second;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    static const D3D11_TEXTURE_ADDRESS_MODE addR[] = {
      D3D11_TEXTURE_ADDRESS_CLAMP,
      D3D11_TEXTURE_ADDRESS_BORDER,
      D3D11_TEXTURE_ADDRESS_MIRROR, //??
      D3D11_TEXTURE_ADDRESS_MIRROR,
      D3D11_TEXTURE_ADDRESS_WRAP,

      D3D11_TEXTURE_ADDRESS_WRAP
      };

    uint32_t filter=0;
    if( s.mipFilter==Texture2d::FilterType::Linear )
      filter |= D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    if( s.magFilter==Texture2d::FilterType::Linear )
      filter |= D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    if( s.minFilter==Texture2d::FilterType::Linear )
      filter |= D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;

    if(s.anisotropic)
      ;//filter=D3D11_FILTER_ANISOTROPIC;

    sampDesc.Filter   = D3D11_FILTER(filter);
    sampDesc.AddressU = addR[ s.uClamp ];
    sampDesc.AddressV = addR[ s.vClamp ];
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MinLOD   = 0;
    sampDesc.MaxLOD   = D3D11_FLOAT32_MAX;

    sampDesc.MaxAnisotropy = 1;

    ID3D11SamplerState* ret = 0;
    HRESULT hr = dev->CreateSamplerState( &sampDesc, &ret );
    if( FAILED(hr) )
      return 0;

    samplers2d[s] = ret;
    return ret;
    }

  HRESULT createShader(ID3DBlob* code, ID3D11VertexShader*& sh ){
    return dev->CreateVertexShader( code->GetBufferPointer(),
                                    code->GetBufferSize(),
                                    NULL,
                                    &sh );
    }

  HRESULT createShader(ID3DBlob* code, ID3D11PixelShader*& sh ){
    return dev->CreatePixelShader( code->GetBufferPointer(),
                                   code->GetBufferSize(),
                                   NULL,
                                   &sh );
    }
   
  template<class Shader>
  bool createShaderFromSource( const std::string &src,
                               std::string &outputLog,
                               Shader& shader,
                               ID3DBlob*& code,
                               const char* model ) {

    ID3DBlob* errors = 0;
    HRESULT result;

    result = D3DCompile( src.data(),
                         src.size(),
                         NULL,            //src-name
                         NULL,            //macro's
                         NULL,            //includes
                         "main",          //main function
                         model,           //shader profile
                         0,               //flags1
                         0,               //flags2
                         &code,           //compiled operations
                         &errors);        //errors
    if( FAILED(result) ){
      if(errors){
        outputLog.insert( outputLog.end(),
                          (char*)errors->GetBufferPointer(),
                          (char*)errors->GetBufferPointer()+errors->GetBufferSize() );
        errors->Release();
        }
      return 0;
      }

    if( errors )
      errors->Release();

    result = createShader( code, shader );

    return SUCCEEDED(result);
    }

  struct Program {
    ID3DBlob*                      codeVs = 0;
    ID3D11VertexShader*            vs     = 0;
    ID3D11PixelShader*             fs     = 0;
    VertexDeclaration::Declarator* decl   = 0;

    ID3D11InputLayout*             dxDecl = 0;

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
  std::shared_ptr<std::vector<ID3D11Buffer*>>            curUboD;
  std::shared_ptr<std::vector<AbstractShadingLang::UBO>> curUbo;
  SortedVec<Program> sh;

  ID3D11InputLayout* createDecl( const VertexDeclaration::Declarator &de,
                                 ID3DBlob* vsBlob ){
    static const DXGI_FORMAT ct[] = {
      DXGI_FORMAT_UNKNOWN,
      DXGI_FORMAT_R32_FLOAT,
      DXGI_FORMAT_R32G32_FLOAT,
      DXGI_FORMAT_R32G32B32_FLOAT,
      DXGI_FORMAT_R32G32B32A32_FLOAT,

      DXGI_FORMAT_R8G8B8A8_UNORM,

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
    ID3D11SamplerState *st = createSampler(s);
    immediateContext->PSSetSamplers( i, 1, &st );
    }

  void setSampler( int i, const Tempest::Texture3d::Sampler& s ){
    (void)i;
    (void)s;
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

void *HLSL11::context() const {
  return data->dev;
  }

void Tempest::HLSL11::bind( const ShaderProgram &px ) const {
  Data::Shader* p = (Data::Shader*)get(px);
  data->prog.vs     = p->vs;
  data->prog.fs     = p->fs;
  data->prog.codeVs = p->blobVs;
  data->prog.dxDecl = 0;

  data->curUboD = p->ubo;
  data->curUbo  = inputOf(px);
  }

void Tempest::HLSL11::setVertexDecl(const AbstractAPI::VertexDecl *d) const {
  data->prog.decl = (VertexDeclaration::Declarator*)d;
  }

AbstractShadingLang::Source
  Tempest::HLSL11::surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                                  bool &hasHalfPixelOffset ) const {
  AbstractShadingLang::Source src;
  src.vs = surfaceShader(Vertex,  opt,hasHalfPixelOffset);
  src.fs = surfaceShader(Fragment,opt,hasHalfPixelOffset);
  return src;
  }

GraphicsSubsystem::ProgramObject*
  Tempest::HLSL11::createShaderFromSource( const AbstractShadingLang::Source &src,
                                           std::string &outputLog ) const {
  if( src.vs.size()==0 &&
      src.fs.size()==0 ){
    outputLog = "no fragment or vertex shader code to compile";
    return 0;
    }

  Data::Shader* prog = new Data::Shader();
  ID3DBlob* compiledFs = 0;
  outputLog.clear();
#ifdef __WINDOWS_PHONE__
  bool sucsess =
    data->createShaderFromSource(src.vs,outputLog,prog->vs,prog->blobVs,"vs_4_0_level_9_1") &&
    data->createShaderFromSource(src.fs,outputLog,prog->fs,compiledFs,  "ps_4_0_level_9_1");
#else
  bool sucsess =
    data->createShaderFromSource(src.vs,outputLog,prog->vs,prog->blobVs,"vs_4_0") &&
    data->createShaderFromSource(src.fs,outputLog,prog->fs,compiledFs,  "ps_4_0");
#endif

  if(compiledFs)
    compiledFs->Release();

  if( !sucsess ){
    delete prog;
    prog=0;
    }

  if(prog)
    prog->ubo.reset( new std::vector<ID3D11Buffer*>() );
  return (GraphicsSubsystem::ProgramObject*)prog;
  }

void Tempest::HLSL11::deleteShader(GraphicsSubsystem::ProgramObject * p) const {
  Data::Shader* prog = (Data::Shader*)p;
  size_t nsz = 0;
  for( size_t i=0; i<data->sh.size(); ++i){
    data->sh[nsz]=data->sh[i];
    if( prog->vs==data->sh[i].vs &&
        prog->fs==data->sh[i].fs ){
      data->sh[i].dxDecl->Release();
      } else {
      ++nsz;
      }
    }
  data->sh.data.resize(nsz);
  delete prog;
  }

void HLSL11::enable() const {
  SortedVec<Data::Program>::iterator l = data->sh.find(data->prog);

  if( l!=data->sh.end() ){
    data->immediateContext->IASetInputLayout(l->dxDecl);
    } else {
    data->prog.dxDecl = data->createDecl(*data->prog.decl, data->prog.codeVs);
    data->sh.insert(data->prog);
    data->immediateContext->IASetInputLayout(data->prog.dxDecl);
    }

  data->immediateContext->VSSetShader( data->prog.vs, NULL, 0 );
  data->immediateContext->PSSetShader( data->prog.fs, NULL, 0 );

  auto ubo = *data->curUbo;

  int slot=0;
  int bufNum = 0;
  if(data->curUboD->size() < ubo.size() )
    data->curUboD->resize(ubo.size());

  ID3D11Buffer** curD = &(*data->curUboD)[0];
  for( const UBO u:ubo ){
    if(!u.updated){
      setUniforms( u, slot );
      u.updated = true;
      if(!curD[bufNum])
        curD[bufNum] = data->createUBO(u.data.size());

      if(curD[bufNum]){
        data->immediateContext->UpdateSubresource( curD[bufNum], 0, 0, &u.data[0], 0, 0 );
        data->immediateContext->VSSetConstantBuffers( bufNum, 1, &curD[bufNum] );
        data->immediateContext->PSSetConstantBuffers( bufNum, 1, &curD[bufNum] );
        }
      }
    ++bufNum;
    }
  }

void Tempest::HLSL11::setUniforms( const AbstractShadingLang::UBO &ux,
                                   int &slot ) const {
  const char*  name      = ux.names.data();
  intptr_t const* fields = ux.fields.data();
  void* const * tex      = ux.tex.data();
  Texture2d::Sampler const* smp2d = (Texture2d::Sampler*)ux.smp[0].data();
  Texture3d::Sampler const* smp3d = (Texture3d::Sampler*)ux.smp[1].data();

  for( int t: ux.desc ){
    //const char* v = &in.data[0] + fields[0];
    ++fields;

    switch(t){
      case Decl::Texture2d:{
        DX11Texture* t = *(DX11Texture**)tex;
        if( t ){
          if(!t->view)
            data->dev->CreateShaderResourceView(t->texture, NULL, &t->view);
          data->immediateContext->PSSetShaderResources(slot,1,&t->view);
          data->setSampler(slot,*smp2d);
          }
        ++slot;
        }
        break;
      case Decl::Texture3d:{
        ++slot;
        }
        break;
      }
    if(t==Decl::Texture2d){
      ++tex;
      ++smp2d;
      } else
    if(t==Decl::Texture3d){
      ++tex;
      ++smp3d;
      }
    name += strlen(name)+1;
    }
  }

std::string HLSL11::surfaceShader( GraphicsSubsystem::ShaderType t,
                                 const AbstractShadingLang::UiShaderOpt &opt,
                                 bool &hasHalfpixOffset ) const {
  hasHalfpixOffset = false;

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

      "Texture2D brush_texture : register(t0);"
      "SamplerState samp:register(s0);"

      "float4 main( FS_Input fs ): SV_Target {"
        "return brush_texture.Sample( samp, float4(fs.tc, 0.0, 0.0) )*fs.cl;"
        "}";


  static const std::string vs_src_nt =
      "struct VS_Input {"
      "  float2 Position:  POSITION;"
      "  float2 TexCoord:  TEXCOORD0;"
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
    data->sh[nsz]=data->sh[i];
    if(e.declaration==(VertexDecl*)data->sh[i].decl){
      data->sh[i].dxDecl->Release();
      } else {
      ++nsz;
      }
    }
  data->sh.data.resize(nsz);
  }
