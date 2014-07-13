#include "directx11.h"

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

using namespace Tempest;

static const IID ID3D11Texture2D_uuid = {0x6f15aaf2,0xd208,0x4e89, {0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};

struct DirectX11::Device{
  ID3D11Device*   device                   = 0;
  D3D_DRIVER_TYPE driverType               = D3D_DRIVER_TYPE_NULL;
  IDXGISwapChain* swapChain                = NULL;
  D3D_FEATURE_LEVEL featureLevel           = D3D_FEATURE_LEVEL_11_0;
  ID3D11DeviceContext* immediateContext    = NULL;
  ID3D11RenderTargetView* renderTargetView = NULL;

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
  Device *dx = (Device*)d;
  return "";//dx->adapter.Description;
  }

AbstractAPI::Device *DirectX11::createDevice(void *Hwnd, const AbstractAPI::Options &/*opt*/) const {
  Device *dev = new Device();

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

  dev->immediateContext->OMSetRenderTargets( 1, &dev->renderTargetView, NULL );

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width    = (FLOAT)width;
  vp.Height   = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  dev->immediateContext->RSSetViewports( 1, &vp );

  return (AbstractAPI::Device*)dev;
  }

void DirectX11::deleteDevice(AbstractAPI::Device *d) const {
  ID3D11Device* dev = Device::dev(d);
  if( dev )
    dev->Release();

  delete ((Device*)d);
  }
