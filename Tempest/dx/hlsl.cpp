#include "hlsl.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <Tempest/Device>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>

using namespace Tempest;

struct HLSL::Data{
  LPDIRECT3DDEVICE9   dev;
  D3DCAPS9            caps;

  struct Prog {
    IDirect3DVertexShader9*    vs = 0;
    ID3DXConstantTable* uniformVs = 0;
    IDirect3DPixelShader9*     fs = 0;
    ID3DXConstantTable* uniformFs = 0;

    ~Prog(){
      if(vs)
        vs->Release();
      if(fs)
        fs->Release();
      if(uniformVs)
        uniformVs->Release();
      if(uniformFs)
        uniformFs->Release();
      }
    };

  Prog*       curProgram;
  const Prog* bindedProg=0;
  std::shared_ptr<std::vector<AbstractShadingLang::UBO>> curUbo;

  HRESULT createShader(LPD3DXBUFFER code, IDirect3DVertexShader9*& sh ){
    return dev->CreateVertexShader( (DWORD*)code->GetBufferPointer(), &sh );
    }

  HRESULT createShader(LPD3DXBUFFER code, IDirect3DPixelShader9*& sh ){
    return dev->CreatePixelShader( (DWORD*)code->GetBufferPointer(), &sh );
    }

  template<class Shader>
  bool createShaderFromSource( const std::string &src,
                               std::string &outputLog,
                               Shader& shader,
                               ID3DXConstantTable*& uniform,
                               const char* model ) {
    LPD3DXBUFFER code = 0, errors = 0;
    HRESULT result;

    result = D3DXCompileShader( src.data(),
                                src.size(),
                                NULL,           //macro's
                                NULL,           //includes
                                "main",         //main function
                                model,          //shader profile
                                0,              //flags
                                &code,          //compiled operations
                                &errors,        //errors
                                &uniform );     //constants
    if( FAILED(result) ){
      if( errors )
        outputLog.insert( outputLog.end(),
                          (char*)errors->GetBufferPointer(),
                          (char*)errors->GetBufferPointer()+errors->GetBufferSize() );
      return 0;
      }
    if( errors )
      errors->Release();

    result = createShader( code, shader );

    return SUCCEEDED(result);
    }

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

void HLSL::bind( const Tempest::ShaderProgram &p ) const {
  data->curProgram = (Data::Prog*)get(p);
  data->curUbo     = inputOf(p);
  }

void *HLSL::context() const {
  return data->dev;
  }

void HLSL::enable() const {
  Data::Prog* prog = data->curProgram;

  auto ubo = *data->curUbo;
  for( const UBO u:ubo )
    setUniforms( prog->uniformVs, u, false );
  for( const UBO u:ubo )
    setUniforms( prog->uniformFs, u, true  );

  if( data->bindedProg==0 ||
      data->bindedProg!=prog ){
    data->bindedProg = prog;
    data->dev->SetVertexShader( prog->vs );
    data->dev->SetPixelShader ( prog->fs );
    }
  }

void HLSL::endPaint() const {
  if( data->curProgram ){
    //UBO& u = inputOf(*data->curProgram);
    //memset( &u.id[0], 0, sizeof(u.id[0])*u.id.size() );
    }
  data->curProgram    = 0;
  data->bindedProg = 0;
  }

template< class Sh>
void HLSL::setUniforms( Sh* prog,
                        const UBO &ux,
                        bool textures ) const {
  int slot=0;

  const char*  name      = ux.names.data();
  intptr_t const* fields = ux.fields.data();
  void* const * tex      = ux.tex.data();
  Texture2d::Sampler* smp2d = (Texture2d::Sampler*)ux.smp[0].data();
  //Texture3d::Sampler* smp3d = (Texture3d::Sampler*)ux.smp[1].data();

  for( int t: ux.desc ){
    D3DXHANDLE prm = prog->GetConstantByName(NULL,name);
    const char* v = &ux.data[0] + fields[0];
    ++fields;

    const float *vec = reinterpret_cast<const float*>(v);
    if(prm!=0)
      switch(t){
        case Decl::float1:
          prog->SetFloat( data->dev, prm, *vec );
          break;
        case Decl::float2:{
          D3DXVECTOR4 v = {vec[0], vec[1], 0, 0};
          prog->SetVector( data->dev, prm, &v );
          }
          break;
        case Decl::float3:{
          D3DXVECTOR4 v = {vec[0], vec[1], vec[2], 0};
          prog->SetVector( data->dev, prm, &v );
          }
          break;
        case Decl::float4:{
          D3DXVECTOR4 v = {vec[0], vec[1], vec[2], vec[3]};
          prog->SetVector( data->dev, prm, &v );
          }
          break;
        case Decl::Texture2d:
          if(textures){
            IDirect3DTexture9* t = *(IDirect3DTexture9**)tex;
            data->setSampler(slot, *smp2d );
            data->dev->SetTexture( slot, t );
            prog->SetInt( data->dev, prm, slot );
            ++slot;
            }
          break;
        case Decl::Texture3d:
          if(textures){
            //IDirect3DTexture9* t = *(IDirect3DTexture9**)tex;
            //data->setSampler(slot, *smp3d );
            //data->dev->SetTexture( slot, t );
            //prog->SetInt( data->dev, prm, slot );
            ++slot;
            }
          break;
        case Decl::Matrix4x4:
          prog->SetMatrix( data->dev, prm, *(D3DXMATRIX**)&v );
          break;
        }
    if(t==Decl::Texture2d){
      ++tex;
      ++smp2d;
      } else
    if(t==Decl::Texture3d){
      ++tex;
      //++smp3d;
      }
    name += strlen(name)+1;
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

      "sampler2D brush_texture;"

      "FS_Output main( FS_Input fs ) {"
        "FS_Output c;"
        "c.color = tex2D(brush_texture, float4(fs.tc, 0.0, 0.0) )*fs.cl;"
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

AbstractShadingLang::Source
  HLSL::surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                       bool &hasHalfPixelOffset ) const {
  AbstractShadingLang::Source src;
  src.vs = surfaceShader(Vertex,  opt,hasHalfPixelOffset);
  src.fs = surfaceShader(Fragment,opt,hasHalfPixelOffset);
  return src;
  }

GraphicsSubsystem::ProgramObject*
  HLSL::createShaderFromSource( const AbstractShadingLang::Source &src,
                                std::string &outputLog ) const {
  if( src.vs.size()==0 &&
      src.fs.size()==0 ){
    outputLog = "no fragment or vertex shader code to compile";
    return 0;
    }

  Data::Prog* prog = new Data::Prog();
  outputLog.clear();
  bool sucsess =
    data->createShaderFromSource(src.vs,outputLog,prog->vs,prog->uniformVs,"vs_3_0") &&
    data->createShaderFromSource(src.fs,outputLog,prog->fs,prog->uniformFs,"ps_3_0");
  if( !sucsess ){
    delete prog;
    prog=0;
    }

  return (GraphicsSubsystem::ProgramObject*)prog;
  }

void HLSL::deleteShader(GraphicsSubsystem::ProgramObject *s) const {
  delete (Data::Prog*)s;
  if(data->curProgram==(Data::Prog*)s){
    data->curProgram = 0;
    data->curUbo.reset();
    data->bindedProg = 0;
    }
  }
