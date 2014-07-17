#include "directx11.h"

#include "shading/abstractshadinglang.h"

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
#include "hlsl11.h"

using namespace Tempest;

static const IID ID3D11Texture2D_uuid = {0x6f15aaf2,0xd208,0x4e89, {0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};

struct DirectX11::Device{
  ID3D11Device*   device                         = 0;
  D3D_DRIVER_TYPE driverType                     = D3D_DRIVER_TYPE_NULL;
  IDXGISwapChain* swapChain                      = NULL;
  D3D_FEATURE_LEVEL featureLevel                 = D3D_FEATURE_LEVEL_11_0;
  ID3D11DeviceContext* immediateContext          = NULL;
  ID3D11RenderTargetView* renderTargetView       = NULL;
  ID3D11DepthStencilView* renderDepthStencilView = NULL;
  ID3D11DepthStencilState* dsState               = NULL;

  ~Device(){
    if( immediateContext )
      immediateContext->ClearState();

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

AbstractAPI::Device *DirectX11::createDevice(void *Hwnd, const AbstractAPI::Options &/*opt*/) const {
  std::unique_ptr<Device> dev( new Device() );

  HRESULT hr   = S_OK;
  HWND    hwnd = (HWND)Hwnd;

  RECT rc;
  GetClientRect( hwnd, &rc );
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  UINT createDeviceFlags = 0;
#ifdef _DEBUG
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

  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof( sd ) );
  sd.BufferCount = 1;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ ) {
    dev->driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDeviceAndSwapChain( NULL, dev->driverType,
                                        NULL, createDeviceFlags,
                                        featureLevels, numFeatureLevels,
                                        D3D11_SDK_VERSION, &sd,
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

  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  descDSV.Format        = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  descDSV.Texture2D.MipSlice = 0;
/*
  // Create the depth stencil view
  hr = pd3dDevice->CreateDepthStencilView( pDepthStencil, // Depth stencil texture
                                           &descDSV, // Depth stencil desc
                                           &dev->renderDepthStencilView );  // [out] Depth stencil view
  if( FAILED( hr ) )
    return 0;

  dev->immediateContext->OMSetRenderTargets( 1, &dev->renderTargetView, NULL );
  */
  /*
  D3D11_DEPTH_STENCIL_DESC dsDesc;
  // Depth test parameters
  dsDesc.DepthEnable    = true;
  dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsDesc.DepthFunc      = D3D11_COMPARISON_LESS;

  // Stencil test parameters
  dsDesc.StencilEnable = true;
  dsDesc.StencilReadMask = 0xFF;
  dsDesc.StencilWriteMask = 0xFF;

  // Stencil operations if pixel is front-facing
  dsDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
  dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
  dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

  // Stencil operations if pixel is back-facing
  dsDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
  dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  dsDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
  dsDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

  // Create depth stencil state
  dev->immediateContext->CreateDepthStencilState(&dsDesc, &dev->dsState);*/

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width    = (FLOAT)width;
  vp.Height   = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  dev->immediateContext->RSSetViewports( 1, &vp );

  return (AbstractAPI::Device*)dev.release();
  }

void DirectX11::deleteDevice(AbstractAPI::Device *d) const {
  delete ((Device*)d);
  }

void DirectX11::clear(AbstractAPI::Device *d, const Color &cl, float z, unsigned stencil) const {
  //TODO
  Device* dev = (Device*)d;

  dev->immediateContext->ClearRenderTargetView( dev->renderTargetView, cl.data() );
  //dev->immediateContext->ClearDepthStencilView( dev->depthTargetView, );
  }

void DirectX11::clear(AbstractAPI::Device *d, const Color &cl) const {
  //TODO
  }

void DirectX11::clear(AbstractAPI::Device *d, const Color &cl, float z) const {
  //TODO
  }

void DirectX11::clear(AbstractAPI::Device *d, float z, unsigned stencil) const {
  //TODO
  }

void DirectX11::clearZ(AbstractAPI::Device *d, float z) const {
  //TODO
  }

void DirectX11::clearStencil(AbstractAPI::Device *d, unsigned stencil) const {
  //TODO
  }

void DirectX11::beginPaint(AbstractAPI::Device *d) const {
  //TODO
  }

void DirectX11::endPaint(AbstractAPI::Device *d) const {
  //TODO
  }

void DirectX11::setRenderState(AbstractAPI::Device *d, const RenderState &) const {
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
  }

void DirectX11::retDSSurfaceTaget(AbstractAPI::Device *d, AbstractAPI::StdDSSurface *s) const {
  //TODO
  }

bool DirectX11::startRender( AbstractAPI::Device *d, bool isLost ) const{
  //TODO
  }

bool DirectX11::present( AbstractAPI::Device *d, SwapBehavior b ) const{
  Device* dev = (Device*)d;
  dev->swapChain->Present(0,0);
  return false;
  }

bool DirectX11::reset( AbstractAPI::Device *d, void* hwnd,
                       const Options & opt ) const{
  //TODO
  }

bool DirectX11::isFormatSupported( AbstractAPI::Device *d, Pixmap::Format f ) const {
  return true;
  }

AbstractAPI::Texture *DirectX11::createTexture(AbstractAPI::Device *d, const Pixmap &p, bool mips, bool compress) const {
  return 0;
  }

AbstractAPI::Texture *DirectX11::recreateTexture( AbstractAPI::Device *d, const Pixmap &p, bool mips, bool compress,
                                                  AbstractAPI::Texture *t) const {
  return 0;
  }

AbstractAPI::Texture *DirectX11::createTexture( AbstractAPI::Device *d, int w, int h, bool mips,
                                                AbstractTexture::Format::Type f, TextureUsage usage ) const {
  return 0;
  }

AbstractAPI::Texture *DirectX11::createTexture3d( AbstractAPI::Device *d, int x, int y, int z, bool mips,
                                                  AbstractTexture::Format::Type f, TextureUsage usage,
                                                  const char *data) const {
  return 0;
  }

void DirectX11::generateMipmaps(AbstractAPI::Device *d, AbstractAPI::Texture *t) const {

  }

void DirectX11::deleteTexture(AbstractAPI::Device *d, AbstractAPI::Texture *t) const {

  }

AbstractAPI::VertexBuffer *DirectX11::createVertexBuffer( AbstractAPI::Device *d,
                                                          size_t size, size_t elSize,
                                                          AbstractAPI::BufferUsage u) const {
  return createVertexBuffer(d,size,elSize,0,u);
  }

AbstractAPI::VertexBuffer *DirectX11::createVertexBuffer( AbstractAPI::Device *d,
                                                          size_t size, size_t elSize,
                                                          void* src,
                                                          AbstractAPI::BufferUsage u) const {
  Device* dev = (Device*)d;
  static const D3D11_USAGE usage[]={
    D3D11_USAGE_STAGING,
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DYNAMIC
    };

  D3D11_BUFFER_DESC bd;
  ZeroMemory( &bd, sizeof(bd) );
  bd.Usage          = usage[u];
  bd.ByteWidth      = elSize*size;
  bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

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
                                                        void * src,
                                                        AbstractAPI::BufferUsage u) const {
  Device* dev = (Device*)d;
  static const D3D11_USAGE usage[]={
    D3D11_USAGE_STAGING,
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DYNAMIC
    };

  D3D11_BUFFER_DESC bd;
  ZeroMemory( &bd, sizeof(bd) );
  bd.Usage          = usage[u];
  bd.ByteWidth      = elSize*size;
  bd.BindFlags      = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;

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

void DirectX11::setVertexDeclaration(AbstractAPI::Device *d, AbstractAPI::VertexDecl *) const {

  }

void DirectX11::bindVertexBuffer(AbstractAPI::Device *d, AbstractAPI::VertexBuffer *, int vsize) const {

  }

void DirectX11::bindIndexBuffer(AbstractAPI::Device *d, AbstractAPI::IndexBuffer *) const {

  }

void *DirectX11::lockBuffer( AbstractAPI::Device *d, AbstractAPI::VertexBuffer *,
                             unsigned offset, unsigned size) const {

  }

void DirectX11::unlockBuffer(AbstractAPI::Device *d, AbstractAPI::VertexBuffer *) const {

  }

void *DirectX11::lockBuffer(AbstractAPI::Device *d, AbstractAPI::IndexBuffer *,
                            unsigned offset, unsigned size) const {

  }

void DirectX11::unlockBuffer(AbstractAPI::Device *d, AbstractAPI::IndexBuffer *) const {

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

  }

void DirectX11::drawIndexed( AbstractAPI::Device *d, AbstractAPI::PrimitiveType t,
                             int vboOffsetIndex, int iboOffsetIndex, int vertexCount) const {

  }

Size DirectX11::windowSize(GraphicsSubsystem::Device *dev) const {
  return Size(0,0);
  }


bool DirectX11::hasManagedStorge() const {
  return true;
  }
