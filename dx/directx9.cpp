#include "directx9.h"

#include <d3d9.h>

#include <d3dx9.h>

#include <Tempest/CgDx9>
#include <Tempest/RenderState>

#include <map>
#include <set>
#include <iostream>
#include <algorithm>

#include <Tempest/Pixmap>
#include <Tempest/Assert>
#include <Tempest/Utility>

using namespace Tempest;

struct DirectX9::Device{
  LPDIRECT3DDEVICE9 dev;
  DirectX9::IBO * curIBO;
  D3DCAPS9        caps;
  D3DADAPTER_IDENTIFIER9 adapter;

  int scrW, scrH;

  bool hasDepthTaget, hasStencilTaget;

  DWORD mkClearFlg(bool cl, bool d, bool s ){
    DWORD re = 0;
    if( cl )
      re |= D3DCLEAR_TARGET;

    if( d && hasDepthTaget )
      re |= D3DCLEAR_ZBUFFER;

    if( s && hasStencilTaget )
      re |= D3DCLEAR_STENCIL;

    return re;
    }
  };

struct DirectX9::IBO{
  LPDIRECT3DINDEXBUFFER9 index;
  std::vector<uint16_t> pbuf;

  struct Min{
    uint16_t val, begin, size;
    };
  std::vector<Min> min;
  };

struct DirectX9::Data{
  static LPDIRECT3DDEVICE9 dev( AbstractAPI::Device* d ){
    return ((DirectX9::Device*)(d))->dev;
    }
  };

DirectX9::DirectX9(){
  impl   = (DirectX9Impl*)Direct3DCreate9( D3D_SDK_VERSION );
  //data   = new Data();
  }

DirectX9::~DirectX9(){
  LPDIRECT3D9 dev = LPDIRECT3D9(impl);
  //delete data;
  dev->Release();
  }

AbstractAPI::Caps DirectX9::caps( AbstractAPI::Device *d ) const {
  Device *dx = (Device*)d;
  Caps c;
  memset( (char*)&c, 0, sizeof(c) );

  c.maxRTCount     = dx->caps.NumSimultaneousRTs;
  c.maxTextureSize = std::min( dx->caps.MaxTextureWidth,
                               dx->caps.MaxTextureHeight );
  c.maxVaryingVectors    = 8;
  c.maxVaryingComponents = c.maxVaryingVectors*4;
  c.hasHalf2 = 1;
  c.hasHalf4 = 1;

  return c;
  }

std::string DirectX9::vendor( AbstractAPI::Device * ) const {
  return "Direct3D";
  }

std::string DirectX9::renderer( AbstractAPI::Device *d ) const {
  Device *dx = (Device*)d;
  return dx->adapter.Description;
  }

bool DirectX9::setDisplaySettings(const DisplaySettings &d) const {
  return AbstractAPI::setDisplaySettings(d);
  }

AbstractAPI::Device* DirectX9::createDevice(void *hwnd, const Options &opt) const {
  LPDIRECT3D9 D3D = LPDIRECT3D9(impl);

  D3DPRESENT_PARAMETERS d3dpp;
  makePresentParams( &d3dpp, hwnd, opt );

  Device* dev = new Device;
  HRESULT derr = D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hwnd,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &dev->dev );

  T_ASSERT_X( !FAILED(derr), "failed to create D3D-Device");

  dev->dev->GetDeviceCaps( &dev->caps );
  D3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &dev->adapter );
  dev->hasDepthTaget   = 1;
  dev->hasStencilTaget = 1;

  RECT rectWindow;
  GetClientRect( HWND(hwnd), &rectWindow);
  dev->scrW = rectWindow.right  - rectWindow.left;
  dev->scrH = rectWindow.bottom - rectWindow.top;

  return (AbstractAPI::Device*)dev;
  }

void DirectX9::makePresentParams( void * p, void *hWnd,
                                  const Options &opt ) const{
  LPDIRECT3D9 D3D = LPDIRECT3D9(impl);

  D3DDISPLAYMODE d3ddm;
  D3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

  D3DPRESENT_PARAMETERS& d3dpp = *(reinterpret_cast<D3DPRESENT_PARAMETERS*>(p));
  ZeroMemory( &d3dpp, sizeof(d3dpp) );

  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
//GetSystemMetrics
  d3dpp.BackBufferWidth  = rectWindow.right  - rectWindow.left;
  d3dpp.BackBufferHeight = rectWindow.bottom - rectWindow.top;

  d3dpp.Windowed               = opt.windowed;
  d3dpp.BackBufferFormat       = d3ddm.Format;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

  d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;

  if( opt.vSync )
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE; else
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;


  if( !d3dpp.Windowed ){
    d3dpp.BackBufferWidth  = d3ddm.Width;
    d3dpp.BackBufferHeight = d3ddm.Height;

    d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
    }
  }

void DirectX9::deleteDevice(AbstractAPI::Device *d) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  if( dev )
    dev->Release();

  Device* de = (Device*)(d);
  delete de;
  }

void DirectX9::clear( AbstractAPI::Device *d,
                      const Color& cl,
                      float z, unsigned stencil ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0, dx->mkClearFlg(1,1,1),
              D3DCOLOR_COLORVALUE( cl.r(), cl.g(), cl.b(), cl.a() ),
              z, stencil );
  }

void DirectX9::clear(AbstractAPI::Device *d, const Color &cl) const  {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0, dx->mkClearFlg(1,0,0),
              D3DCOLOR_COLORVALUE( cl.r(), cl.g(), cl.b(), cl.a() ),
              0, 0 );
  }

void DirectX9::clearZ( AbstractAPI::Device *d, float z ) const  {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0,
              dx->mkClearFlg(0,1,0),
              0,
              z, 0 );
  }

void DirectX9::clearStencil( AbstractAPI::Device *d, unsigned s ) const  {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0,
              dx->mkClearFlg(0,0,1),
              0,
              0, s );
  }

void DirectX9::clear( AbstractAPI::Device *d,
                      float z, unsigned s ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0,
              dx->mkClearFlg(0,1,1),
              0,
              z, s );
  }

void DirectX9::clear( AbstractAPI::Device *d, const Color& cl, float z ) const{
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  Device *dx = (Device*)d;

  dev->Clear( 0, 0,
              dx->mkClearFlg(1,1,0),
              D3DCOLOR_COLORVALUE( cl.r(), cl.g(), cl.b(), cl.a() ),
              z, 0 );
  }

void DirectX9::beginPaint( AbstractAPI::Device *d ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);

  dev->BeginScene();
  }

void DirectX9::endPaint  ( AbstractAPI::Device *d ) const{
  LPDIRECT3DDEVICE9 dev = Data::dev(d);

  dev->EndScene();
  }

void DirectX9::setRenderTaget( AbstractAPI::Device *d,
                               AbstractAPI::Texture   *t, int mip,
                               int mrtSlot ) const {
  T_ASSERT_X(t, "null render taget");
  LPDIRECT3DDEVICE9  dev  = Data::dev(d);
  LPDIRECT3DTEXTURE9 tex  = LPDIRECT3DTEXTURE9(t);

  LPDIRECT3DSURFACE9 surf = 0;
  tex->GetSurfaceLevel( mip, &surf);
  //UINT c = surf->Release();

  //for (int i=0; i<16; i++)
    //dev->SetTexture(i, 0);

  dev->SetRenderTarget(mrtSlot, surf);
  tex->Release();
  }

void DirectX9::unsetRenderTagets( AbstractAPI::Device *d,
                                  int count  ) const {
  LPDIRECT3DDEVICE9  dev  = Data::dev(d);

  IDirect3DSurface9* backBuf = 0;
  dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuf);
  dev->SetRenderTarget( 0, backBuf );
  backBuf->Release();

  for( int i=1; i<count; ++i)
    dev->SetRenderTarget( i, 0 );
  }

AbstractAPI::StdDSSurface *DirectX9::getDSSurfaceTaget( AbstractAPI::Device *d ) const {
  LPDIRECT3DDEVICE9  dev  = Data::dev(d);
  LPDIRECT3DSURFACE9 surf = 0;

  if( FAILED(dev->GetDepthStencilSurface(&surf)) )
    return 0;

  return (AbstractAPI::StdDSSurface*)(surf);
  }

void DirectX9::retDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface *s ) const {
  Device*  dev  = (Device*)(d);
  LPDIRECT3DSURFACE9 surf = LPDIRECT3DSURFACE9(s);

  if( surf )
    surf->Release();

  dev->hasDepthTaget   = 1;
  dev->hasStencilTaget = 1;
  }

void DirectX9::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::StdDSSurface *tx ) const {
  LPDIRECT3DDEVICE9  dev  = Data::dev(d);


  LPDIRECT3DSURFACE9 surf = LPDIRECT3DSURFACE9(tx);
  //LPDIRECT3DTEXTURE9 tex  = LPDIRECT3DTEXTURE9(tx);
  //tex->GetSurfaceLevel( 0, &surf );

  dev->SetDepthStencilSurface( surf );

  {
    Device*  dev  = (Device*)(d);
    dev->hasDepthTaget   = surf;
    dev->hasStencilTaget = surf;
    }
  //surf->Release();
  }

void DirectX9::setDSSurfaceTaget( AbstractAPI::Device *d,
                                  AbstractAPI::Texture *tx ) const {
  LPDIRECT3DDEVICE9  dev  = Data::dev(d);

  if( tx ){
    LPDIRECT3DSURFACE9 surf = 0;
    LPDIRECT3DTEXTURE9 tex  = LPDIRECT3DTEXTURE9(tx);
    tex->GetSurfaceLevel( 0, &surf );
    dev->SetDepthStencilSurface( surf );

    tex->Release();
    } else {
    dev->SetDepthStencilSurface(0);
    }

  {
    Device*  dev  = (Device*)(d);
    dev->hasDepthTaget   = tx;
    dev->hasStencilTaget = tx;
    }
  }

bool DirectX9::startRender( AbstractAPI::Device *d,
                            bool isLost  ) const {

  if( isLost ){
    Sleep( 100 );

    LPDIRECT3DDEVICE9 dev = Data::dev(d);
    return (dev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET);
    }

  return 1;
  }

bool DirectX9::present(AbstractAPI::Device *d, SwapBehavior /*b*/ ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);

  return ( D3DERR_DEVICELOST == dev->Present( NULL, NULL, NULL, NULL ) );
  }

bool DirectX9::reset( AbstractAPI::Device *d, void* hwnd,
                      const Options &opt ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);

  D3DPRESENT_PARAMETERS d3dpp;
  makePresentParams( &d3dpp, hwnd, opt );

  //return 1;

  HRESULT hr = dev->Reset( &d3dpp );

  if( FAILED(hr ) )
    return 0; else
    return 1;
  }

AbstractAPI::Texture *DirectX9::createTexture( AbstractAPI::Device *d,
                                               const Pixmap &p,
                                               bool mips,
                                               bool compress ) const {
  if( p.width()==0 || p.height()==0 )
    return 0;

  D3DLOCKED_RECT lockedRect;
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  LPDIRECT3DTEXTURE9 tex = 0;

  static const D3DFORMAT frm[] = {
    D3DFMT_X8R8G8B8,
    D3DFMT_A8R8G8B8,
    D3DFMT_DXT1,
    D3DFMT_DXT3,
    D3DFMT_DXT5
    };
  //D3DFORMAT format = frm[p.format()];

  if( !(p.format()==Pixmap::Format_RGB ||
        p.format()==Pixmap::Format_RGBA) ){
    int mipCount = 0;

    if( mips ){
      int w = std::max(1, p.width() ),
          h = std::max(1, p.height());
      while( w>1||h>1){
        if(w>1) w/=2;
        if(h>1) h/=2;
        ++mipCount;
        }
      }

    if (FAILED(dev->CreateTexture( p.width(), p.height(), mipCount,
                                   0, frm[p.format()],
                                   D3DPOOL_MANAGED,
                                   &tex, NULL))) {
      return 0;
      }
    } else {
    if (FAILED(dev->CreateTexture( p.width(), p.height(), 0,
                                   D3DUSAGE_AUTOGENMIPMAP, frm[p.format()],
                                   D3DPOOL_MANAGED,
                                   &tex, NULL))) {
      return 0;
      }
    }

  if( p.format()==Pixmap::Format_RGB ||
      p.format()==Pixmap::Format_RGBA ){
    if (FAILED( tex->LockRect(0, &lockedRect, 0, 0))) {
      return 0;
      }

    unsigned char *dest = (unsigned char*) lockedRect.pBits;
    for( int i=0; i<p.width(); ++i )
      for( int r=0; r<p.height(); ++r ){
        unsigned char * t = &dest[ 4*(i + r*p.width()) ];
        const Pixmap::Pixel s = p.at(i, p.height()-r-1 );
        t[2] = s.r;
        t[1] = s.g;
        t[0] = s.b;
        t[3] = s.a;
        }

    tex->UnlockRect(0);
    if( mips )
      tex->GenerateMipSubLevels();
    } else {
    int nBlockSize = 16;
    if( p.format() == Pixmap::Format_DXT1 )
        nBlockSize = 8;
    const unsigned char *mipData = p.const_data();
    int w = p.width(), h = p.height();

    for(int i=0; w>1 || h>1; ++i ){
      if (FAILED( tex->LockRect(i, &lockedRect, 0, 0))) {
        return 0;
        }
      int nSize = ((w+3)/4) * ((h+3)/4) * nBlockSize;
      memcpy( lockedRect.pBits, mipData, nSize );
      tex->UnlockRect(0);

      if( w>1 ) w/=2;
      if( h>1 ) h/=2;
      mipData += nSize;

      if( !mips ){
        return ((AbstractAPI::Texture*)tex);
        }
      }
    }

  //data->tex.insert( tex );
  return ((AbstractAPI::Texture*)tex);
  }

AbstractAPI::Texture *DirectX9::recreateTexture( AbstractAPI::Device *d,
                                                 AbstractAPI::Texture *oldT,
                                                 const Pixmap &p,
                                                 bool mips,
                                                 bool compress ) const {
  if( oldT==0 )
    return createTexture(d, p, mips, compress);

  D3DLOCKED_RECT lockedRect;
  LPDIRECT3DDEVICE9 dev  = Data::dev(d);
  LPDIRECT3DTEXTURE9 tex = 0;
  LPDIRECT3DTEXTURE9 old = LPDIRECT3DTEXTURE9(oldT);

  static const D3DFORMAT frm[] = {
    D3DFMT_X8R8G8B8,
    D3DFMT_A8R8G8B8,
    D3DFMT_DXT1,
    D3DFMT_DXT3,
    D3DFMT_DXT5
    };

  D3DSURFACE_DESC desc;
  if( FAILED( old->GetLevelDesc(0, &desc) ) ){
    deleteTexture(d, oldT);
    return 0;
    }

  if( int(desc.Width)==p.width() &&
      int(desc.Height)==p.height() &&
      desc.Format==frm[p.format()] ){
    tex = old;
    } else {
    deleteTexture(d, oldT);

    if( !(p.format()==Pixmap::Format_RGB ||
          p.format()==Pixmap::Format_RGBA) ){
      int mipCount = 0;

      if( mips ){
        int w = std::max(1, p.width() ),
            h = std::max(1, p.height());
        while( w>1||h>1){
          if(w>1) w/=2;
          if(h>1) h/=2;
          ++mipCount;
          }
        }

      if (FAILED(dev->CreateTexture( p.width(), p.height(), mipCount,
                                     0, frm[p.format()],
                                     D3DPOOL_MANAGED,
                                     &tex, NULL))) {
        return 0;
        }
      } else {
      if (FAILED(dev->CreateTexture( p.width(), p.height(), 0,
                                     D3DUSAGE_AUTOGENMIPMAP, frm[p.format()],
                                     D3DPOOL_MANAGED,
                                     &tex, NULL))) {
        return 0;
        }
      }
    }

  if( p.format()==Pixmap::Format_RGB ||
      p.format()==Pixmap::Format_RGBA ){
    if (FAILED( tex->LockRect(0, &lockedRect, 0, 0))) {
      return 0;
      }
    int bpp = 3;
    if( p.format()==Pixmap::Format_RGBA )
      bpp = 4;

    unsigned char *dest = (unsigned char*) lockedRect.pBits;
    const unsigned char *src  = p.const_data();

    for( int i=0; i<p.width(); ++i )
      for( int r=0; r<p.height(); ++r ){
        unsigned char       * t = &dest[   4*(i + r*p.width()) ];
        const unsigned char * s =  &src[ bpp*(i + (p.height()-r-1)*p.width()) ];
        /*
        const Pixmap::Pixel s = p.at(i,r);
        t[2] = s.r;
        t[1] = s.g;
        t[0] = s.b;
        t[3] = s.a;*/
        t[2] = s[0];
        t[1] = s[1];
        t[0] = s[2];

        t[3] = (bpp==4)?s[3]:255;
        }    
    tex->UnlockRect(0);
    if( mips )
      tex->GenerateMipSubLevels();
    } else {
    int nBlockSize = 16;
    if( p.format() == Pixmap::Format_DXT1 )
        nBlockSize = 8;
    const unsigned char *mipData = p.const_data();
    int w = p.width(), h = p.height();

    for(int i=0; w>1 || h>1; ++i ){
      if (FAILED( tex->LockRect(i, &lockedRect, 0, 0))) {
        return 0;
        }
      int nSize = ((w+3)/4) * ((h+3)/4) * nBlockSize;
      memcpy( lockedRect.pBits, mipData, nSize );
      tex->UnlockRect(0);

      if( w>1 ) w/=2;
      if( h>1 ) h/=2;
      mipData += nSize;

      if( !mips ){
        return ((AbstractAPI::Texture*)tex);
        }
      }
    }

  //data->tex.insert( tex );
  return ((AbstractAPI::Texture*)tex);
  }

AbstractAPI::Texture* DirectX9::createTexture(AbstractAPI::Device *d,
                                               int w, int h,
                                               bool mips,
                                               AbstractTexture::Format::Type f,
                                               TextureUsage u  ) const {
  LPDIRECT3DDEVICE9 dev  = Data::dev(d);
  LPDIRECT3DTEXTURE9 tex = 0;

  static const DWORD usage[] = {
    D3DUSAGE_RENDERTARGET,
    D3DUSAGE_RENDERTARGET,
    D3DUSAGE_DYNAMIC,
    0
    };

  static const D3DFORMAT d3frm[] = {
    D3DFMT_L16,  // Luminance,
    D3DFMT_A4L4, // Luminance4,
    D3DFMT_L8,   // Luminance8,
    D3DFMT_L16,  // Luminance16,

    D3DFMT_R8G8B8,   // RGB,
    D3DFMT_X4R4G4B4, // RGB4,
    D3DFMT_R5G6B5, // RGB5,
    D3DFMT_A2B10G10R10, // RGB10,
    D3DFMT_A2B10G10R10, // RGB12,
    D3DFMT_A2B10G10R10, // RGB16,

    D3DFMT_A8R8G8B8,     // RGBA,
    D3DFMT_A1R5G5B5,     // RGBA5,
    D3DFMT_A8R8G8B8,     // RGBA8,
    D3DFMT_A2B10G10R10,  // RGB10_A2,
    D3DFMT_A2B10G10R10,  // RGBA12,
    D3DFMT_A16B16G16R16, // RGBA16,

    D3DFMT_DXT1, // RGB_DXT1,
    D3DFMT_DXT3, // RGBA_DXT1,
    D3DFMT_DXT3, // RGBA_DXT3,
    D3DFMT_DXT5, // RGBA_DXT5,

    D3DFMT_D16,   // Depth16,
    D3DFMT_D24X8, // Depth24,
    D3DFMT_D32,   // Depth32,
    D3DFMT_G16R16,// RG16
    D3DFMT_A8R8G8B8, //Count
    };

  if( f==AbstractTexture::Format::Depth16 ||
      f==AbstractTexture::Format::Depth24 ||
      f==AbstractTexture::Format::Depth32 ){
/*
    LPDIRECT3DSURFACE9 surf = 0;

    HRESULT hr = dev->CreateDepthStencilSurface( w, h, d3frm[f],
                                                 D3DMULTISAMPLE_NONE,
                                                 0, TRUE, &surf, NULL);
    return ((AbstractAPI::Texture*)surf);*/

    HRESULT hr = D3DXCreateTexture(  dev,
                                     w,
                                     h,
                                     1,
                                     D3DUSAGE_DEPTHSTENCIL,//usage[u],

                                     d3frm[f],
                                     D3DPOOL_DEFAULT,
                                     &tex );

    if( FAILED( hr ) ){
      return 0;
      }
    return ((AbstractAPI::Texture*)tex);
    //data->tex.insert( surf );
    } else {
    int ww = std::max(w,h)/2, mipsc = 1;
    while( mips && ww>1 ){
      ww/=2;
      ++mipsc;
      }

    if( FAILED(
      D3DXCreateTexture(  dev,
                          w,
                          h,
                          mipsc,
                          usage[u],

                          d3frm[f],
                          D3DPOOL_DEFAULT,
                          &tex ) ) ){
      return 0;
      }

    //data->tex.insert( tex );
    return ((AbstractAPI::Texture*)tex);
    }
  }

void DirectX9::deleteTexture( AbstractAPI::Device *,
                              AbstractAPI::Texture *t ) const {
  IUnknown *tex = (IUnknown*)(t);
  if( tex ){
    /*ULONG count = */tex->Release();
    //std::cout << "DirectX9::deleteTexture " << count << std::endl;
    }
  }

AbstractAPI::VertexBuffer*
     DirectX9::createVertexBuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize,
                                   BufferUsage /*usage*/) const{
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  LPDIRECT3DVERTEXBUFFER9 vbo = 0;

  int c = 100;
  // avoid false out of memory alarm
  while( !vbo && c>0 ){
    if( FAILED( dev->CreateVertexBuffer( size*elSize,
                                         D3DUSAGE_WRITEONLY,
                                         0,
                                         D3DPOOL_DEFAULT,
                                         &vbo,
                                         0 ) ) ){
        T_ASSERT(0);
        vbo = 0;
        }
    --c;
    }

  return ((AbstractAPI::VertexBuffer*)vbo);
  }

void DirectX9::deleteVertexBuffer( AbstractAPI::Device *,
                                    AbstractAPI::VertexBuffer*v ) const{
  LPDIRECT3DVERTEXBUFFER9 vbo = LPDIRECT3DVERTEXBUFFER9(v);
  if( vbo )
    vbo->Release();
  }

AbstractAPI::IndexBuffer*
     DirectX9::createIndexBuffer( AbstractAPI::Device *d,
                                  size_t size, size_t elSize,
                                  BufferUsage /*usage*/ ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  LPDIRECT3DINDEXBUFFER9 index = 0;

  int c = 100;
  // avoid false out of memory alarm
  while( !index && c>0 ){
    if( FAILED( dev->CreateIndexBuffer( size*elSize,
                                        D3DUSAGE_WRITEONLY,
                                        D3DFMT_INDEX16,
                                        D3DPOOL_DEFAULT,
                                        &index,
                                        0 ) ) ){
        T_ASSERT(0);
        index = 0;
        }
    --c;
    }

  if( index==0 )
    return 0;

  IBO * ibo = new IBO();

  ibo->pbuf.reserve( size );
  ibo->pbuf.resize( size );
  ibo->index = index;

  return ((AbstractAPI::IndexBuffer*)ibo);
  }

void DirectX9::deleteIndexBuffer( AbstractAPI::Device *,
                                  AbstractAPI::IndexBuffer* b) const {
  IBO* ibo = (IBO*)(b);
  if( ibo && ibo->index )
    ibo->index->Release();

  delete ibo;
  }

void* DirectX9::lockBuffer( AbstractAPI::Device *,
                            AbstractAPI::VertexBuffer * v,
                            unsigned offset,
                            unsigned /*size*/ ) const {
  LPDIRECT3DVERTEXBUFFER9 vbo = LPDIRECT3DVERTEXBUFFER9(v);

  void* pVertices = 0;
  T_ASSERT( vbo->Lock( 0, 0, &pVertices, 0 )==D3D_OK );

  return (char*)pVertices+offset;
  }

void DirectX9::unlockBuffer( AbstractAPI::Device *,
                             AbstractAPI::VertexBuffer* v) const {
  LPDIRECT3DVERTEXBUFFER9 vbo = LPDIRECT3DVERTEXBUFFER9(v);
  vbo->Unlock();
  }

void* DirectX9::lockBuffer( AbstractAPI::Device *,
                            AbstractAPI::IndexBuffer * v,
                            unsigned offset, unsigned /*size*/) const {
  IBO* ibo = (IBO*)(v);

  char* ptr = (char*)&ibo->pbuf[0];
  return ptr+offset;
  }

void DirectX9::unlockBuffer( AbstractAPI::Device *,
                             AbstractAPI::IndexBuffer* v) const {
  IBO* ibo = (IBO*)(v);
  void *pIBO = 0;

  size_t byteSize = ibo->pbuf.size()*sizeof(ibo->pbuf[0]);

  T_ASSERT( ibo->index->Lock( 0, 0, &pIBO, 0 )==D3D_OK );
  memcpy(pIBO, &ibo->pbuf[0], byteSize );
  ibo->index->Unlock();

  ibo->min.clear();
  }

const AbstractShadingLang*
        DirectX9::createShadingLang( AbstractAPI::Device *d ) const {
  AbstractAPI::DirectX9Device* dev =
      (AbstractAPI::DirectX9Device*)reinterpret_cast<DirectX9::Device*>(d)->dev;

  return new CgDx9( dev );
  }

void DirectX9::deleteShadingLang( const AbstractShadingLang * l ) const {
  delete l;
  }

AbstractAPI::VertexDecl *
      DirectX9::createVertexDecl( AbstractAPI::Device *d,
                                  const VertexDeclaration::Declarator &de ) const {
  BYTE ct[] = {
    D3DDECLTYPE_FLOAT1,
    D3DDECLTYPE_FLOAT2,
    D3DDECLTYPE_FLOAT3,
    D3DDECLTYPE_FLOAT4,

    D3DDECLTYPE_D3DCOLOR,

    D3DDECLTYPE_SHORT2,
    D3DDECLTYPE_SHORT4,

    D3DDECLTYPE_FLOAT16_2,
    D3DDECLTYPE_FLOAT16_4
    };

  BYTE usage[] = {
    D3DDECLUSAGE_POSITION,
    D3DDECLUSAGE_BLENDWEIGHT,   // 1
    D3DDECLUSAGE_BLENDINDICES,  // 2
    D3DDECLUSAGE_NORMAL,        // 3
    D3DDECLUSAGE_PSIZE,         // 4
    D3DDECLUSAGE_TEXCOORD,      // 5
    D3DDECLUSAGE_TANGENT,       // 6
    D3DDECLUSAGE_BINORMAL,      // 7
    D3DDECLUSAGE_TESSFACTOR,    // 8
    D3DDECLUSAGE_POSITIONT,     // 9
    D3DDECLUSAGE_COLOR,         // 10
    D3DDECLUSAGE_FOG,           // 11
    D3DDECLUSAGE_DEPTH,         // 12
    D3DDECLUSAGE_SAMPLE,        // 13
    };
  /*
  D3DVERTEXELEMENT9 declaration[] =
  {
      { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      { 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
      D3DDECL_END()
  };*/

  std::vector<D3DVERTEXELEMENT9> decl;
  for( int i=0; i<de.size(); ++i ){
    D3DVERTEXELEMENT9 e;
    e.Stream = 0;
    e.Offset = 4*de[i].component;
    e.Type   = ct[ de[i].component-1 ];

    if( de[i].component==Decl::color )
      e.Offset = 4;

    if( de[i].component==Decl::short2 ||
        de[i].component==Decl::half2 )
      e.Offset = 4;

    if( de[i].component==Decl::short4 ||
        de[i].component==Decl::half4 )
      e.Offset = 8;

    e.Method = D3DDECLMETHOD_DEFAULT;
    e.Usage = usage[ de[i].usage ];
    e.UsageIndex = de[i].index;

    decl.push_back(e);
    }

  for( size_t i=1; i<decl.size(); ++i ){
    decl[i].Offset += decl[i-1].Offset;
    }

  if( decl.size() ){
    for( size_t i=decl.size()-1; i>=1; --i ){
      decl[i].Offset = decl[i-1].Offset;
      }

    decl[0].Offset = 0;
    }

  D3DVERTEXELEMENT9 endEl = D3DDECL_END();
  decl.push_back( endEl );

  // cgD3D9ValidateVertexDeclaration( g_CGprogram_vertex, declaration );

  LPDIRECT3DVERTEXDECLARATION9 ret = 0;

  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  HRESULT hr = dev->CreateVertexDeclaration( decl.data(), &ret );

  if( FAILED(hr) ){
    T_ASSERT(0);
    }

  return reinterpret_cast<AbstractAPI::VertexDecl*>(ret);
  }

void DirectX9::deleteVertexDecl( AbstractAPI::Device *,
                                 AbstractAPI::VertexDecl* de ) const {
  LPDIRECT3DVERTEXDECLARATION9  decl = LPDIRECT3DVERTEXDECLARATION9(de);
  if( decl )
    decl->Release();
  }

void DirectX9::setVertexDeclaration( AbstractAPI::Device *d,
                                     AbstractAPI::VertexDecl* de ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  LPDIRECT3DVERTEXDECLARATION9  decl = LPDIRECT3DVERTEXDECLARATION9(de);

  dev->SetFVF(0);
  dev->SetVertexDeclaration( decl );
  }

void DirectX9::bindVertexBuffer( AbstractAPI::Device *d,
                                 AbstractAPI::VertexBuffer* b,
                                 int vsize ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);

  HRESULT re =
  dev->SetStreamSource( 0, LPDIRECT3DVERTEXBUFFER9(b),
                        0, vsize );
  T_ASSERT(re==D3D_OK);
  }

void DirectX9::bindIndexBuffer( AbstractAPI::Device * d,
                                AbstractAPI::IndexBuffer * b ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev(d);
  HRESULT re = dev->SetIndices( ((IBO*)(b))->index );

  ((Device*)d)->curIBO = (IBO*)(b);
  T_ASSERT(re==D3D_OK);
  }

void DirectX9::draw( AbstractAPI::Device *d,
                     AbstractAPI::PrimitiveType t,
                     int firstVertex, int pCount ) const {
  LPDIRECT3DDEVICE9 dev = Data::dev( d );

  static const D3DPRIMITIVETYPE type[] = {
    D3DPT_POINTLIST,//             = 1,
    D3DPT_LINELIST,//              = 2,
    D3DPT_LINESTRIP,//             = 3,
    D3DPT_TRIANGLELIST,//          = 4,
    D3DPT_TRIANGLESTRIP,//         = 5,
    D3DPT_TRIANGLEFAN,//           = 6
    };

  //int vpCount = vertexCount(t, pCount);
  dev->DrawPrimitive( type[ t-1 ], firstVertex, pCount );
  }

void DirectX9::drawIndexed( AbstractAPI::Device *d,
                            AbstractAPI::PrimitiveType t,
                            int vboOffsetIndex,
                            int iboOffsetIndex,
                            int pCount ) const {
  Device *dev = (Device*)( d );

  static const D3DPRIMITIVETYPE type[] = {
    D3DPT_POINTLIST,//             = 1,
    D3DPT_LINELIST,//              = 2,
    D3DPT_LINESTRIP,//             = 3,
    D3DPT_TRIANGLELIST,//          = 4,
    D3DPT_TRIANGLESTRIP,//         = 5,
    D3DPT_TRIANGLEFAN,//           = 6
    };

  int vpCount = vertexCount(t, pCount);
  uint16_t minID = -1;
  for( size_t i=0; i<dev->curIBO->min.size(); ++i ){
    IBO::Min& m = dev->curIBO->min[i];
    if( m.begin==iboOffsetIndex && m.size==vpCount )
      minID = m.val;
    }

  if( minID == uint16_t(-1) ){
    IBO::Min m;
    m.begin = iboOffsetIndex;
    m.size  = vpCount;
    m.val   = *std::min_element( dev->curIBO->pbuf.begin()+iboOffsetIndex,
                                 dev->curIBO->pbuf.begin()+iboOffsetIndex+vpCount );
    minID = m.val;
    dev->curIBO->min.push_back(m);
    }

  dev->dev->DrawIndexedPrimitive( type[ t-1 ],
                                  vboOffsetIndex,
                                  minID,
                                  vpCount,
                                  iboOffsetIndex,
                                  pCount );
  }

Size DirectX9::windowSize( Tempest::AbstractAPI::Device *d) const {
  Device *dev = (Device*)( d );

  return Size(dev->scrW, dev->scrH);
  }

void DirectX9::setRenderState( AbstractAPI::Device *d,
                               const RenderState & r) const {
  LPDIRECT3DDEVICE9 dev = Data::dev( d );

  static const D3DCULL cull[] = {
    D3DCULL_NONE,
    D3DCULL_CW,
    D3DCULL_CCW
    };

  static const D3DCMPFUNC cmp[] = {
    D3DCMP_NEVER,

    D3DCMP_GREATER,
    D3DCMP_LESS,

    D3DCMP_GREATEREQUAL,
    D3DCMP_LESSEQUAL,

    D3DCMP_NOTEQUAL,
    D3DCMP_EQUAL,
    D3DCMP_ALWAYS,
    D3DCMP_ALWAYS
    };

  dev->SetRenderState( D3DRS_CULLMODE, cull[ r.cullFaceMode() ]);

  dev->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
  D3DCMPFUNC f = cmp[ r.getZTestMode() ] ;
  if( !r.isZTest() )
    f = D3DCMP_ALWAYS;

  dev->SetRenderState( D3DRS_ZFUNC,        f );
  dev->SetRenderState( D3DRS_ZWRITEENABLE, r.isZWriting() );

  if( r.alphaTestMode()!=RenderState::AlphaTestMode::Always ){
    dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    dev->SetRenderState( D3DRS_ALPHAFUNC, cmp[ r.alphaTestMode() ] );
    dev->SetRenderState( D3DRS_ALPHAREF, 255*r.alphaTestRef() );
    } else {
    dev->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    }

  static const D3DBLEND  blend[] = {
    D3DBLEND_ZERO,         //zero,                 //GL_ZERO,
    D3DBLEND_ONE,          //one,                  //GL_ONE,
    D3DBLEND_SRCCOLOR,     //src_color,            //GL_SRC_COLOR,
    D3DBLEND_INVSRCCOLOR,  //GL_ONE_MINUS_SRC_COLOR,
    D3DBLEND_SRCALPHA,     //GL_SRC_ALPHA,
    D3DBLEND_INVSRCALPHA,  //GL_ONE_MINUS_SRC_ALPHA,
    D3DBLEND_DESTALPHA,    //GL_DST_ALPHA,
    D3DBLEND_INVDESTALPHA, //GL_ONE_MINUS_DST_ALPHA,
    D3DBLEND_DESTCOLOR,    //GL_DST_COLOR,
    D3DBLEND_INVDESTCOLOR, //GL_ONE_MINUS_DST_COLOR,
    D3DBLEND_SRCALPHASAT,  //GL_SRC_ALPHA_SATURATE,
    D3DBLEND_ZERO
    };

  if( r.isBlend() ){
    dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    dev->SetRenderState( D3DRS_SRCBLEND,  blend[ r.getBlendSFactor() ] );
    dev->SetRenderState( D3DRS_DESTBLEND, blend[ r.getBlendDFactor() ] );
    } else {
    dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    }

  bool w[4];
  r.getColorMask( w[0], w[1], w[2], w[3] );

  DWORD flg = 0;
  if( w[0] )
    flg |= D3DCOLORWRITEENABLE_RED;
  if( w[1] )
    flg |= D3DCOLORWRITEENABLE_GREEN;
  if( w[2] )
    flg |= D3DCOLORWRITEENABLE_BLUE;
  if( w[3] )
    flg |= D3DCOLORWRITEENABLE_ALPHA;

  dev->SetRenderState( D3DRS_COLORWRITEENABLE, flg );

  D3DFILLMODE fm[3] = {
    D3DFILL_SOLID,
    D3DFILL_WIREFRAME,
    D3DFILL_POINT
    };

  dev->SetRenderState( D3DRS_FILLMODE, fm[ r.frontPolygonRenderMode() ] );
  }
