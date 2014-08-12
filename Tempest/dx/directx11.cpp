#include "directx11.h"

#include "shading/abstractshadinglang.h"
#include <Tempest/RenderState>
#include <Tempest/Platform>

#ifdef __WINDOWS_PHONE__
#include <win_rt_api_binding.h>
#include <DXGI1_2.h>
#endif

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
#include "dx11types.h"
#include "hlsl11.h"

#include <unordered_map>

using namespace Tempest;

static const IID ID3D11Texture2D_uuid = {0x6f15aaf2,0xd208,0x4e89, {0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};

struct BlendDesc{
  D3D11_BLEND d,s;
  bool        blend;
  };

struct ZDesc{
  D3D11_COMPARISON_FUNC func;
  bool enable, writing;
  };

struct Hash {
  size_t operator () ( const BlendDesc& a ) const {
    return a.blend ? size_t(a.d+1) : 0;
    }
  size_t operator () ( const ZDesc& a ) const {
    return a.enable ? size_t(a.func+1) : 0;
    }
  };

struct Cmp {
  bool operator () ( const BlendDesc& a, const BlendDesc& b ) const {
    return a.blend==b.blend && a.d==b.d && a.s==b.s;
    }
  bool operator () ( const ZDesc& a, const ZDesc& b ) const {
    return a.enable==b.enable && a.writing==b.writing && a.func==b.func;
    }
  };

#ifdef __WINDOWS_PHONE__
typedef IDXGISwapChain1       SwapChain;
typedef DXGI_SWAP_CHAIN_DESC1 SWAP_CHAIN_DESC;
#else
typedef IDXGISwapChain        SwapChain;
typedef DXGI_SWAP_CHAIN_DESC  SWAP_CHAIN_DESC;
#endif

struct DirectX11::Device{
  ID3D11Device*            device           = 0;
  D3D_DRIVER_TYPE          driverType       = D3D_DRIVER_TYPE_NULL;
  SwapChain*               swapChain        = NULL;
  D3D_FEATURE_LEVEL        featureLevel     = D3D_FEATURE_LEVEL_11_0;
  ID3D11DeviceContext*     immediateContext = NULL;
  ID3D11RenderTargetView*  renderTargetView = NULL;
  ID3D11Texture2D*         depthStencil     = NULL;
  ID3D11DepthStencilView*  depthStencilView = NULL;
  ID3D11DepthStencilState* dsState          = NULL;

  std::unordered_map<BlendDesc,ID3D11BlendState*,Hash,Cmp> blendSt;
  std::unordered_map<ZDesc,ID3D11DepthStencilState*,Hash,Cmp> ztestSt;

  int scrW, scrH;

  ~Device(){
    if( immediateContext )
      immediateContext->ClearState();

    if( depthStencilView )
      depthStencilView->Release();
    if( depthStencil )
      depthStencil->Release();

    if( renderTargetView )
      renderTargetView->Release();
    if( swapChain )
      swapChain->Release();

    if( immediateContext )
      immediateContext->Release();
    if( device )
      device->Release();
    }

  Tempest::AbstractAPI::Caps caps;

  static ID3D11Device* dev(void* d){
    return ((Device*)d)->device;
    }

  std::vector<uint8_t> texFlipped;
  void* flipTexture(int w, int h, int bpp, const uint8_t * pix){
    if( texFlipped.size()<size_t(w*h*4) )
      texFlipped.resize(w*h*4);

    unsigned char* tx = &texFlipped[0];
    if(bpp==4){
      for( int r=0; r<h; ++r ){
        memcpy( &tx[r*w*bpp], pix+w*(h-r-1)*bpp, w*bpp );
        }
      } else {
      for( int r=0; r<h; ++r )
        for(int i=0; i<w; ++i){
          const int r1 = h-r-1;
          memcpy( &tx[(r*w+i)*4], pix+(w*r1+i)*3, 3 );
          tx[(r*w+i)*4+3] = 255;
          }
      }

    return &texFlipped[0];
    }

  HRESULT createDepthTexture( int width, int height ) {
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width  = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D( &descDepth, NULL, &depthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = device->CreateDepthStencilView( depthStencil, &descDSV, &depthStencilView );
    if( FAILED( hr ) )
        return hr;
    return S_OK;
    }
  };

DirectX11::DirectX11() {
  }

DirectX11::~DirectX11() {
  }

AbstractAPI::Caps DirectX11::caps( AbstractAPI::Device *d) const {
  return ((Device*)d)->caps;
  }

std::string DirectX11::vendor( AbstractAPI::Device * ) const {
  return "Direct3D";
  }

std::string DirectX11::renderer( AbstractAPI::Device *d ) const {
  //TODO
  Device *dx = (Device*)d;
  return "";//dx->adapter.Description;
  }

static HRESULT createDevice( IDXGIAdapter *pAdapter,
                             D3D_DRIVER_TYPE driverType,
                             HMODULE software,
                             UINT flags,
                             const D3D_FEATURE_LEVEL *pFeatureLevels,
                             UINT featureLevels,
                             const SWAP_CHAIN_DESC *swapChainDesc,
                             SwapChain **ppSwapChain,
                             ID3D11Device **ppDevice,
                             D3D_FEATURE_LEVEL *pFeatureLevel,
                             ID3D11DeviceContext **ppImmediateContext ){
#ifdef __WINDOWS_PHONE__
  HRESULT hr = D3D11CreateDevice(
    pAdapter,
    driverType,
    0,
    flags,
    pFeatureLevels,
    featureLevels,
    D3D11_SDK_VERSION,
    ppDevice,
    pFeatureLevel,
    ppImmediateContext
    );

  if( FAILED(hr) ){
    return hr;
    }

  ID3D11Device* device = *ppDevice;
  IDXGIDevice2 * pDXGIDevice;
  hr = device->QueryInterface(__uuidof(IDXGIDevice2), (void **)&pDXGIDevice );

  IDXGIAdapter * pDXGIAdapter;
  hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

  IDXGIFactory2 * pIDXGIFactory;
  pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&pIDXGIFactory);

  IUnknown* mainWidget = (IUnknown*)WinRt::getMainRtWidget();
  return pIDXGIFactory->CreateSwapChainForCoreWindow( device,
                                                      mainWidget,
                                                      swapChainDesc,
                                                      nullptr,
                                                      ppSwapChain
                                                      );
#else
  return D3D11CreateDeviceAndSwapChain( pAdapter,
                                        driverType,
                                        software,
                                        flags,
                                        pFeatureLevels, featureLevels,
                                        D3D11_SDK_VERSION, swapChainDesc,
                                        ppSwapChain,
                                        ppDevice,
                                        pFeatureLevel,
                                        ppImmediateContext );
#endif
  }

AbstractAPI::Device *DirectX11::createDevice(void *Hwnd, const AbstractAPI::Options &/*opt*/) const {
  std::unique_ptr<Device> dev( new Device() );

  HRESULT hr   = S_OK;
  HWND    hwnd = (HWND)Hwnd;

#ifdef __WINDOWS_PHONE__
  WinRt::getScreenRect(hwnd,dev->scrW,dev->scrH);
#else
  RECT rc;
  GetClientRect( hwnd, &rc );
  UINT width  = rc.right  - rc.left;
  UINT height = rc.bottom - rc.top;

  dev->scrW = width;
  dev->scrH = height;
#endif

  UINT createDeviceFlags = 0;
#ifndef NDEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] =
  {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE,
  };
  UINT numDriverTypes = ARRAYSIZE( driverTypes );

  D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1,
  };
  UINT numFeatureLevels = ARRAYSIZE( featureLevels );

  SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof( sd ) );
#ifdef __WINDOWS_PHONE__
  sd.Width  = dev->scrW; // Match the size of the window.
  sd.Height = dev->scrH;
  sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
  sd.Stereo = false;
  sd.SampleDesc.Count   = 1; // Don't use multi-sampling.
  sd.SampleDesc.Quality = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = 2; // Use double-buffering to minimize latency.
  sd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
  sd.Flags       = 0;
  sd.Scaling     = DXGI_SCALING_NONE;
  sd.AlphaMode   = DXGI_ALPHA_MODE_IGNORE;
#else
  sd.BufferCount = 2;
  sd.SwapEffect  = DXGI_SWAP_EFFECT_SEQUENTIAL;
  sd.BufferDesc.Width  = dev->scrW;
  sd.BufferDesc.Height = dev->scrH;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
#endif

  for( UINT driverTypeIndex = 0; driverTypeIndex<numDriverTypes; driverTypeIndex++ ) {
    dev->driverType = driverTypes[driverTypeIndex];
    hr = ::createDevice( NULL, dev->driverType,
                         NULL, createDeviceFlags,
                         featureLevels, numFeatureLevels,
                         &sd,
                         &dev->swapChain,
                         &dev->device,
                         &dev->featureLevel,
                         &dev->immediateContext );
    if( SUCCEEDED( hr ) )
      break;
    }

  if( FAILED( hr ) )
    return 0;

  ID3D11Texture2D* pBackBuffer = NULL;
  hr = dev->swapChain->GetBuffer( 0, ID3D11Texture2D_uuid, ( LPVOID* )&pBackBuffer );
  if( FAILED( hr ) )
    return 0;

  hr = dev->device->CreateRenderTargetView( pBackBuffer, NULL, &dev->renderTargetView );
  pBackBuffer->Release();
  if( FAILED( hr ) )
    return 0;

  if( FAILED(dev->createDepthTexture(dev->scrW,dev->scrH)) )
    return 0;
  dev->immediateContext->OMSetRenderTargets( 1, &dev->renderTargetView,
                                                 dev->depthStencilView );

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width    = (FLOAT)dev->scrW;
  vp.Height   = (FLOAT)dev->scrH;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  dev->immediateContext->RSSetViewports( 1, &vp );

  dev->caps.maxTextureSize = 8048;
  return (AbstractAPI::Device*)dev.release();
  }

void DirectX11::deleteDevice(AbstractAPI::Device *d) const {
  Device* dev = (Device*)d;

  for( auto i:dev->blendSt )
    if(i.second)
      i.second->Release();

  for( auto i:dev->ztestSt )
    if(i.second)
      i.second->Release();

  delete ((Device*)d);
  }

void DirectX11::clear( AbstractAPI::Device *d, const Color &cl,
                       float z, unsigned stencil ) const {
  Device* dev = (Device*)d;

  dev->immediateContext->ClearRenderTargetView( dev->renderTargetView, cl.data() );
  dev->immediateContext->ClearDepthStencilView( dev->depthStencilView,
                                                D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
                                                z, stencil );
  }

void DirectX11::clear(AbstractAPI::Device *d, const Color &cl) const {
  Device* dev = (Device*)d;
  dev->immediateContext->ClearRenderTargetView( dev->renderTargetView, cl.data() );
  }

void DirectX11::clear(AbstractAPI::Device *d, const Color &cl, float z) const {
  Device* dev = (Device*)d;

  dev->immediateContext->ClearRenderTargetView( dev->renderTargetView, cl.data() );
  dev->immediateContext->ClearDepthStencilView( dev->depthStencilView,
                                                D3D11_CLEAR_DEPTH,
                                                z, 0 );
  }

void DirectX11::clear(AbstractAPI::Device *d, float z, unsigned stencil) const {
  Device* dev = (Device*)d;
  dev->immediateContext->ClearDepthStencilView( dev->depthStencilView,
                                                D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
                                                z, stencil );
  }

void DirectX11::clearZ(AbstractAPI::Device *d, float z) const {
  Device* dev = (Device*)d;
  dev->immediateContext->ClearDepthStencilView( dev->depthStencilView,
                                                D3D11_CLEAR_DEPTH,
                                                z, 0 );
  }

void DirectX11::clearStencil(AbstractAPI::Device *d, unsigned stencil) const {
  Device* dev = (Device*)d;
  dev->immediateContext->ClearDepthStencilView( dev->depthStencilView,
                                                D3D11_CLEAR_STENCIL,
                                                1.0f, stencil );
  }

void DirectX11::beginPaint(AbstractAPI::Device *d) const {
  //TODO
  }

void DirectX11::endPaint(AbstractAPI::Device *d) const {
  //TODO
  }

void DirectX11::setRenderTaget(AbstractAPI::Device *d, AbstractAPI::Texture *tx, int mip, int mrtSlot) const {
  //TODO
  }

void DirectX11::unsetRenderTagets(AbstractAPI::Device *d, int count) const {
  //TODO
  }

void DirectX11::setDSSurfaceTaget(AbstractAPI::Device *d, AbstractAPI::StdDSSurface *tx) const {
  //TODO
  }

void DirectX11::setDSSurfaceTaget(AbstractAPI::Device *d, AbstractAPI::Texture *tx) const {
  //TODO
  }

AbstractAPI::StdDSSurface *DirectX11::getDSSurfaceTaget(AbstractAPI::Device *d) const {
  //TODO
  return 0;
  }

void DirectX11::retDSSurfaceTaget(AbstractAPI::Device *d, AbstractAPI::StdDSSurface *s) const {
  //TODO
  }

bool DirectX11::startRender( AbstractAPI::Device *, bool /*isLost*/ ) const{
  return true;
  }

bool DirectX11::present( AbstractAPI::Device *d, SwapBehavior /*b*/ ) const{
  Device* dev = (Device*)d;
  dev->swapChain->Present(1,0);
  return false;
  }

bool DirectX11::reset( AbstractAPI::Device *d, void* hwnd,
                       const Options & /*opt*/ ) const{
  Device* dx = (Device*)d;
#ifdef __WINDOWS_PHONE__
  WinRt::getScreenRect( hwnd, dx->scrW, dx->scrH );
#else
  RECT rc;
  GetClientRect( (HWND)hwnd, &rc );
  dx->scrW = rc.right  - rc.left;
  dx->scrH = rc.bottom - rc.top;
#endif

  if(dx->swapChain){
    dx->immediateContext->OMSetRenderTargets(0, 0, 0);

    // Release all outstanding references to the swap chain's buffers.
    dx->renderTargetView->Release();
    dx->depthStencilView->Release();
    dx->depthStencil->Release();

    HRESULT hr=0;
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = dx->swapChain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,0);
    if(FAILED(hr))
      return false;

    ID3D11Texture2D* pBuffer;
    hr = dx->swapChain->GetBuffer(0,ID3D11Texture2D_uuid,(void**)&pBuffer );
    if(FAILED(hr))
      return false;

    hr = dx->device->CreateRenderTargetView(pBuffer,NULL,&dx->renderTargetView);
    if(FAILED(hr))
      return false;
    pBuffer->Release();

    dx->createDepthTexture(dx->scrW,dx->scrH);
    dx->immediateContext->OMSetRenderTargets( 1,
                                              &dx->renderTargetView,
                                              dx->depthStencilView );

    D3D11_VIEWPORT vp;
    vp.Width    = FLOAT(dx->scrW);
    vp.Height   = FLOAT(dx->scrH);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    dx->immediateContext->RSSetViewports( 1, &vp );
    }

  return true;
  }

bool DirectX11::isFormatSupported( AbstractAPI::Device *, Pixmap::Format f ) const {
  return f==Pixmap::Format_RGB || f==Pixmap::Format_RGBA;
  }

AbstractAPI::Texture *DirectX11::createTexture( AbstractAPI::Device *d,
                                                const Pixmap &p,
                                                bool mips,
                                                bool /*compress*/ ) const {
  Device* dev = (Device*)d;
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.ArraySize        = 1;
  desc.Width            = p.width();
  desc.Height           = p.height();
  desc.MipLevels        = 1;//desc.ArraySize = 1;
  desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.Usage            = D3D11_USAGE_DEFAULT;//D3D11_USAGE_IMMUTABLE;
  desc.BindFlags        = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags   = 0;
  desc.MiscFlags        = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  desc.SampleDesc.Count   = 1;
  desc.SampleDesc.Quality = 0;

  D3D11_SUBRESOURCE_DATA pix;
  ZeroMemory(&pix, sizeof(pix));
  const int bpp   = p.hasAlpha() ? 4:3;
  pix.pSysMem     = dev->flipTexture(p.width(), p.height(), bpp, p.const_data());
  pix.SysMemPitch = p.width()*4;

  DX11Texture* tex = new DX11Texture;
  if( SUCCEEDED(dev->device->CreateTexture2D( &desc, &pix, &tex->texture )) ){
    dev->device->CreateShaderResourceView(tex->texture, NULL, &tex->view);
    if(mips)
      dev->immediateContext->GenerateMips(tex->view);
    return (AbstractAPI::Texture*)tex;
    }
  return 0;
  }

AbstractAPI::Texture *DirectX11::recreateTexture( AbstractAPI::Device *d,
                                                  const Pixmap &p,
                                                  bool mips,
                                                  bool compress,
                                                  AbstractAPI::Texture *t) const {
  deleteTexture(d,t);
  return createTexture(d,p,mips,compress);
  }

AbstractAPI::Texture* DirectX11::createTexture( AbstractAPI::Device *d,
                                                int w, int h, bool mips,
                                                AbstractTexture::Format::Type f,
                                                TextureUsage usage ) const {
  static const D3D11_USAGE u[] = {
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DEFAULT,
    D3D11_USAGE_DEFAULT,
    D3D11_USAGE_DEFAULT//D3D11_USAGE_STAGING
    };

  Device* dev = (Device*)d;
  D3D11_TEXTURE2D_DESC desc;
  desc.ArraySize        = 1;
  desc.Width            = w;
  desc.Height           = h;
  desc.MipLevels        = 1;//desc.ArraySize = 1;
  desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.Usage            = u[usage];
  desc.BindFlags        = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags   = 0;
  desc.MiscFlags        = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  desc.SampleDesc.Count   = 1;
  desc.SampleDesc.Quality = 0;

  DX11Texture* tex = new DX11Texture;
  if( SUCCEEDED(dev->device->CreateTexture2D( &desc, NULL, &tex->texture )) ){
    dev->device->CreateShaderResourceView(tex->texture, NULL, &tex->view);
    if(mips)
      dev->immediateContext->GenerateMips(tex->view);
    return (AbstractAPI::Texture*)tex;
    }
  return 0;
  }

AbstractAPI::Texture *DirectX11::createTexture3d( AbstractAPI::Device *d,
                                                  int x, int y, int z, bool mips,
                                                  AbstractTexture::Format::Type f,
                                                  TextureUsage usage,
                                                  const char *data) const {
  return 0;
  }

void DirectX11::generateMipmaps(AbstractAPI::Device *d, AbstractAPI::Texture *t) const {
  Device* dev = (Device*)d;
  DX11Texture* tex = (DX11Texture*)t;

  if(tex->view==NULL)
    dev->device->CreateShaderResourceView(tex->texture, NULL, &tex->view);
  dev->immediateContext->GenerateMips(tex->view);
  }

void DirectX11::deleteTexture(AbstractAPI::Device *, AbstractAPI::Texture *t) const {
  DX11Texture* tex = (DX11Texture*)t;
  if( tex->texture )
    tex->texture->Release();
  if( tex->view )
    tex->view->Release();
  }

AbstractAPI::VertexBuffer *DirectX11::createVertexBuffer( AbstractAPI::Device *d,
                                                          size_t size, size_t elSize,
                                                          AbstractAPI::BufferUsage u) const {
  return createVertexBuffer(d,size,elSize,0,u);
  }

AbstractAPI::VertexBuffer *DirectX11::createVertexBuffer( AbstractAPI::Device *d,
                                                          size_t size,
                                                          size_t elSize,
                                                          const void *src,
                                                          AbstractAPI::BufferUsage u) const {
  Device* dev = (Device*)d;
  static const D3D11_USAGE usage[]={
    D3D11_USAGE_DYNAMIC,
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DYNAMIC
    };

  D3D11_BUFFER_DESC bd;
  ZeroMemory( &bd, sizeof(bd) );
  bd.Usage          = usage[u];
  bd.ByteWidth      = elSize*size;
  bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  if(bd.Usage==D3D11_USAGE_DYNAMIC)
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA initData;
  ZeroMemory( &initData, sizeof(initData) );
  initData.pSysMem = src;

  ID3D11Buffer* buffer = 0;
  HRESULT hr = dev->device->CreateBuffer( &bd, &initData, &buffer );

  if( FAILED( hr ) )
    return 0;

  return (AbstractAPI::VertexBuffer*)buffer;
  }

void DirectX11::deleteVertexBuffer( AbstractAPI::Device *,
                                    AbstractAPI::VertexBuffer *b) const {
  ID3D11Buffer* buffer = (ID3D11Buffer*)b;
  buffer->Release();
  }

AbstractAPI::IndexBuffer *DirectX11::createIndexBuffer( AbstractAPI::Device *d,
                                                        size_t size, size_t elSize,
                                                        AbstractAPI::BufferUsage u) const {
  return createIndexBuffer(d,size,elSize,0,u);
  }

AbstractAPI::IndexBuffer *DirectX11::createIndexBuffer( AbstractAPI::Device *d,
                                                        size_t size, size_t elSize,
                                                        const void * src,
                                                        AbstractAPI::BufferUsage u) const {
  Device* dev = (Device*)d;
  static const D3D11_USAGE usage[]={
    D3D11_USAGE_DYNAMIC,
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DYNAMIC
    };

  D3D11_BUFFER_DESC bd;
  ZeroMemory( &bd, sizeof(bd) );
  bd.Usage          = usage[u];
  bd.ByteWidth      = elSize*size;
  bd.BindFlags      = D3D11_BIND_INDEX_BUFFER;
  if(bd.Usage==D3D11_USAGE_DYNAMIC)
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA initData;
  ZeroMemory( &initData, sizeof(initData) );
  initData.pSysMem = src;

  ID3D11Buffer* buffer = 0;
  HRESULT hr = dev->device->CreateBuffer( &bd, &initData, &buffer );

  if( FAILED( hr ) )
    return 0;

  return (AbstractAPI::IndexBuffer*)buffer;
  }

void DirectX11::deleteIndexBuffer(AbstractAPI::Device*, AbstractAPI::IndexBuffer *b) const {
  ID3D11Buffer* buffer = (ID3D11Buffer*)b;
  buffer->Release();
  }

AbstractAPI::VertexDecl*
  DirectX11::createVertexDecl( AbstractAPI::Device *,
                               const VertexDeclaration::Declarator &de ) const {
  return (AbstractAPI::VertexDecl*)(new VertexDeclaration::Declarator(de));
  }

void DirectX11::deleteVertexDecl( AbstractAPI::Device *,
                                  AbstractAPI::VertexDecl *decl ) const {
  VertexDeclaration::Declarator *d = (VertexDeclaration::Declarator*)decl;
  delete d;
  }

void DirectX11::setVertexDeclaration(AbstractAPI::Device *,
                                     AbstractAPI::VertexDecl *) const {
  }//NOP

void DirectX11::bindVertexBuffer( AbstractAPI::Device *d,
                                  AbstractAPI::VertexBuffer *v, int vsize) const {

  Device* dev = (Device*)d;
  ID3D11Buffer* b = (ID3D11Buffer*)v;
  UINT stride=vsize, offset=0;
  dev->immediateContext->IASetVertexBuffers( 0, 1, &b, &stride, &offset );
  }

void DirectX11::bindIndexBuffer( AbstractAPI::Device *d,
                                 AbstractAPI::IndexBuffer *v ) const {
  Device* dev = (Device*)d;
  ID3D11Buffer* b = (ID3D11Buffer*)v;
  dev->immediateContext->IASetIndexBuffer(b, DXGI_FORMAT_R16_UINT, 0 );
  }

void *DirectX11::lockBuffer( AbstractAPI::Device *d,
                             AbstractAPI::VertexBuffer *b,
                             unsigned offset,
                             unsigned size ) const {
  Device* dev = (Device*)d;
  ID3D11Buffer* buf = (ID3D11Buffer*)b;

  D3D11_MAPPED_SUBRESOURCE resource;
  D3D11_BUFFER_DESC desc;
  buf->GetDesc(&desc);

  HRESULT hResult = dev->immediateContext->Map( (ID3D11Buffer*)b, 0,
                                                D3D11_MAP_WRITE_DISCARD,
                                                0, &resource);
  if(hResult != S_OK)
     return 0;
  memset(resource.pData,0,desc.ByteWidth);//FIXME
  return (char*)resource.pData + offset;
  }

void DirectX11::unlockBuffer( AbstractAPI::Device *d,
                              AbstractAPI::VertexBuffer *b ) const {
  Device* dev = (Device*)d;
  dev->immediateContext->Unmap((ID3D11Buffer*)b, 0);
  }

void *DirectX11::lockBuffer(AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer *b,
                            unsigned offset, unsigned /*size*/) const {
  Device* dev = (Device*)d;
  D3D11_MAPPED_SUBRESOURCE resource;
  HRESULT hResult = dev->immediateContext->Map( (ID3D11Buffer*)b, 0,
                                                D3D11_MAP_WRITE_DISCARD, 0, &resource);
  if(hResult != S_OK)
     return 0;
  return (char*)resource.pData + offset;
  }

void DirectX11::unlockBuffer(AbstractAPI::Device *d, AbstractAPI::IndexBuffer *b) const {
  Device* dev = (Device*)d;
  dev->immediateContext->Unmap((ID3D11Buffer*)b, 0);
  }

AbstractShadingLang *DirectX11::createShadingLang(AbstractAPI::Device *d) const {
  Device* dev = (Device*)d;
  return new HLSL11( (DirectX11Device*)dev->device, dev->immediateContext );
  }

void DirectX11::deleteShadingLang(const AbstractShadingLang *l) const {
  delete l;
  }

void DirectX11::draw( AbstractAPI::Device *d, AbstractAPI::PrimitiveType t,
                      int firstVertex, int pCount) const {
  static const D3D_PRIMITIVE_TOPOLOGY topology[]={
    D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP//deprecated
    };

  const int vpCount = vertexCount(t, pCount);
  Device* dev = (Device*)d;
  dev->immediateContext->IASetPrimitiveTopology( topology[t] );
  dev->immediateContext->Draw( vpCount, firstVertex );
  }

void DirectX11::drawIndexed( AbstractAPI::Device *d, AbstractAPI::PrimitiveType t,
                             int vboOffsetIndex, int iboOffsetIndex, int pCount) const {
  static const D3D_PRIMITIVE_TOPOLOGY topology[]={
    D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP//deprecated
    };

  const int vpCount = vertexCount(t, pCount);
  Device* dev = (Device*)d;
  dev->immediateContext->IASetPrimitiveTopology( topology[t] );
  dev->immediateContext->DrawIndexed( vpCount, iboOffsetIndex, vboOffsetIndex );
  }

Size DirectX11::windowSize(GraphicsSubsystem::Device *d) const {
  Device* dev = (Device*)d;
  return Size(dev->scrW, dev->scrH);
  }


bool DirectX11::hasManagedStorge() const {
  return true;
  }

void DirectX11::setRenderState( AbstractAPI::Device *d,
                                const RenderState &rs ) const {
  static const D3D11_BLEND blend[] = {
    D3D11_BLEND_ZERO,           //zero,                 //GL_ZERO,
    D3D11_BLEND_ONE,            //one,                  //GL_ONE,
    D3D11_BLEND_SRC_COLOR,      //src_color,            //GL_SRC_COLOR,
    D3D11_BLEND_INV_SRC_COLOR,  //GL_ONE_MINUS_SRC_COLOR,
    D3D11_BLEND_SRC_ALPHA,      //GL_SRC_ALPHA,
    D3D11_BLEND_INV_SRC_ALPHA,  //GL_ONE_MINUS_SRC_ALPHA,
    D3D11_BLEND_DEST_ALPHA,     //GL_DST_ALPHA,
    D3D11_BLEND_INV_DEST_ALPHA, //GL_ONE_MINUS_DST_ALPHA,
    D3D11_BLEND_DEST_COLOR,     //GL_DST_COLOR,
    D3D11_BLEND_INV_DEST_COLOR, //GL_ONE_MINUS_DST_COLOR,
    D3D11_BLEND_SRC_ALPHA_SAT,  //GL_SRC_ALPHA_SATURATE,
    D3D11_BLEND_ZERO
    };

  Device* dev = (Device*)d;
  ID3D11BlendState* state = 0;

  BlendDesc bd;
  bd.blend = rs.isBlend();
  bd.d     = blend[rs.getBlendDFactor()];
  bd.s     = blend[rs.getBlendSFactor()];

  auto i=dev->blendSt.find(bd);
  if(i!=dev->blendSt.end()){
    dev->immediateContext->OMSetBlendState(i->second, 0, 0xffffffff);
    } else {
    D3D11_BLEND_DESC BlendState;
    ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
    BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    BlendState.RenderTarget[0].BlendEnable    = rs.isBlend();
    BlendState.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    BlendState.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;

    BlendState.RenderTarget[0].SrcBlend       = blend[rs.getBlendSFactor()];
    BlendState.RenderTarget[0].SrcBlendAlpha  = blend[rs.getBlendSFactor()];
    BlendState.RenderTarget[0].DestBlend      = blend[rs.getBlendDFactor()];
    BlendState.RenderTarget[0].DestBlendAlpha = blend[rs.getBlendDFactor()];

    for( int i=1; i<8; ++i )
      BlendState.RenderTarget[i] = BlendState.RenderTarget[0];

    dev->device->CreateBlendState(&BlendState, &state);
    dev->blendSt[bd] = state;
    dev->immediateContext->OMSetBlendState(state, 0, 0xffffffff);
    }

  static D3D11_COMPARISON_FUNC func[]={
    D3D11_COMPARISON_NEVER,

    D3D11_COMPARISON_GREATER,
    D3D11_COMPARISON_LESS,

    D3D11_COMPARISON_GREATER_EQUAL,
    D3D11_COMPARISON_LESS_EQUAL,

    D3D11_COMPARISON_NOT_EQUAL,
    D3D11_COMPARISON_EQUAL,
    D3D11_COMPARISON_ALWAYS,
    D3D11_COMPARISON_ALWAYS
    };
  ZDesc zdesc;
  zdesc.enable  = rs.isZTest();
  zdesc.writing = rs.isZWriting();
  zdesc.func    = func[rs.getZTestMode()];

  auto zi=dev->ztestSt.find(zdesc);
  if(zi!=dev->ztestSt.end()){
    dev->immediateContext->OMSetDepthStencilState(zi->second,0);
    } else {
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable    = rs.isZTest();
    dsDesc.DepthWriteMask = rs.isZWriting() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc      = func[rs.getZTestMode()];

    dsDesc.StencilEnable    = false;
    dsDesc.StencilReadMask  = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    dsDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    dsDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    // Create depth stencil state
    ID3D11DepthStencilState * dsState=0;
    HRESULT h = dev->device->CreateDepthStencilState(&dsDesc, &dsState);
    T_ASSERT( SUCCEEDED(h) );
    dev->immediateContext->OMSetDepthStencilState(dsState,0);
    dev->ztestSt[zdesc] = dsState;
    }
  //TODO
  }
