#include "windowsapi.h"

#ifdef __WIN32__
#include <windows.h>

#include <Tempest/Window>
#include <Tempest/Event>

#include <unordered_map>
#include <fstream>
#include <cassert>

#include <IL/il.h>
#include "core/wrappers/atomic.h"

using namespace Tempest;

static std::unordered_map<WindowsAPI::Window*, Tempest::Window*> wndWx;

static LRESULT CALLBACK WindowProc( HWND   hWnd,
                                    UINT   msg,
                                    WPARAM wParam,
                                    LPARAM lParam );

WindowsAPI::WindowsAPI() {
  }

WindowsAPI::~WindowsAPI() {
  }

void WindowsAPI::startApplication(ApplicationInitArgs *) {
  WNDCLASSEX winClass;

  winClass.lpszClassName = L"Tempest_Window_Class";
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW;
  winClass.lpfnWndProc   = WindowProc;
  winClass.hInstance     = GetModuleHandle(0);
  winClass.hIcon         = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hIconSm       = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  if( !RegisterClassEx(&winClass) )
    ;//return E_FAIL;
  }

void WindowsAPI::endApplication() {
  UnregisterClass( L"Tempest_Window_Class", GetModuleHandle(0) );
  }

int WindowsAPI::nextEvent(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));

  if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
    if( uMsg.message==WM_QUIT )
      quit = 1;

    TranslateMessage( &uMsg );
    DispatchMessage ( &uMsg );
    return uMsg.wParam;
    } else {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      i->second->render();

    return 0;
    }
  }

WindowsAPI::Window *WindowsAPI::createWindow(int w, int h) {
  HWND hwnd = CreateWindowEx( NULL,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              0, 0, w, h, NULL, NULL,
                              GetModuleHandle(0), NULL );
  //SetWindowLong( hwnd, DWL_USER, this );

  return (Window*)hwnd;
  }

void WindowsAPI::deleteWindow( Window *w ) {
  DestroyWindow( (HWND)w );
  wndWx.erase(w);
  }

void WindowsAPI::show(Window *w) {
  HWND hwnd = (HWND)w;

  ShowWindow( hwnd, SW_SHOW );
  UpdateWindow( hwnd );
  }

void WindowsAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  LONG lStyles = GetWindowLong( (HWND)hw, GWL_STYLE );

  if( lStyles & WS_MINIMIZE )
    return;

  if( lStyles & WS_MAXIMIZE )
    return;

  MoveWindow( (HWND)hw, x, y, w, h, false );
  }

void WindowsAPI::bind( Window *w, Tempest::Window *wx ) {
  wndWx[w] = wx;
  }

std::string WindowsAPI::loadTextImpl(const char *file) {
  std::ifstream is( file, std::ifstream::binary );
  assert(is);

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  std::string src;
  src.resize( length );
  is.read ( &src[0], length );

  assert(is);
  is.close();

  return src;
  }

void WindowsAPI::initImgLib() {
  Tempest::Detail::Atomic::begin();

  static bool wasInit = false;
  if( !wasInit )
    ilInit();
  ilEnable(IL_FILE_OVERWRITE);

  wasInit = true;

  Tempest::Detail::Atomic::end();
  }

template< class ChanelType, int mul >
void WindowsAPI::initRawData( std::vector<unsigned char> &d,
                              void * input,
                              int bpp,
                              int w,
                              int h,
                              int * ix ){
  ChanelType * img = reinterpret_cast<ChanelType*>(input);

  d.resize( bpp*w*h );

  for( int i=0; i<w; ++i )
    for( int r=0; r<h; ++r ){
      ChanelType * raw = &img[ bpp*(i+r*w) ];
      for( int q=0; q<bpp; ++q ){
        d[ bpp*(i+r*w)+q ] = ( raw[ ix[q] ]*mul );
        }

      }
  }

bool WindowsAPI::loadImageImpl( const char *file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char> &out ) {
  initImgLib();
  bool ok = true;

  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  if( ilLoadImage ( file ) ){
    int format = ilGetInteger( IL_IMAGE_FORMAT );

    int size_of_pixel = 1;
    bpp = 3;

    int idx[5][4] = {
      { 0, 1, 2, 3 }, //IL_RGB, IL_RGBA
      { 0, 0, 0, 0 }, //IL_ALPHA
      { 2, 1, 0, 3 },
      { 2, 1, 0, 3 },
      { 0, 0, 0, 1 }
      };
    int *ix = idx[0];


    switch( format ){
      case IL_RGB:   size_of_pixel = 3; break;
      case IL_RGBA:  size_of_pixel = 4; bpp = 4; break;
      case IL_ALPHA: size_of_pixel = 1; ix = idx[1]; break;

      case IL_BGR:   size_of_pixel = 3; ix = idx[2]; break;
      case IL_BGRA:  size_of_pixel = 4; ix = idx[3]; break;
      case IL_LUMINANCE_ALPHA:
        size_of_pixel = 4; ix = idx[4];
        break;
      default: ok = false;
      }

    if( ok ){
      ILubyte * data = ilGetData();
      //Data * image = new Data();

      w = ilGetInteger ( IL_IMAGE_WIDTH  );
      h = ilGetInteger ( IL_IMAGE_HEIGHT );

      if( ilGetInteger ( IL_IMAGE_TYPE ) == IL_UNSIGNED_BYTE )
        initRawData<ILubyte,   1> ( out, data, size_of_pixel, w, h, ix ); else
        initRawData<ILdouble, 255>( out, data, size_of_pixel, w, h, ix );
      }
    } else {
    ok = false;
    }

  ilDeleteImages( 1, &id );

  return ok;
  }

bool WindowsAPI::saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& in ){
  initImgLib();

  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  ilTexImage( w, h, 1,
              bpp,
              (bpp==4)? IL_RGBA : IL_RGB,
              IL_UNSIGNED_BYTE,
              in.data() );

  ILubyte * img = ilGetData();

  int h2 = h/2;
  for( int i=0; i<w; ++i )
    for( int r=0; r<h2; ++r ){
      ILubyte * raw1 = &img[ bpp*(i+r*w) ];
      ILubyte * raw2 = &img[ bpp*(i+(h-r-1)*w) ];

      for( int q=0; q<bpp; ++q )
        std::swap( raw1[q], raw2[q] );
      }

  bool ok = ilSaveImage( file );
  ilDeleteImages( 1, &id );

  return ok;
  }

static Event::MouseButton toButton( UINT msg ){
  if( msg==WM_LBUTTONDOWN ||
      msg==WM_LBUTTONUP )
    return Event::ButtonLeft;

  if( msg==WM_RBUTTONDOWN  ||
      msg==WM_RBUTTONUP)
    return Event::ButtonRight;

  if( msg==WM_MBUTTONDOWN ||
      msg==WM_MBUTTONUP )
    return Event::ButtonMid;

  return Event::ButtonNone;
  }

LRESULT CALLBACK WindowProc( HWND   hWnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam ) {
    //return DefWindowProc( hWnd, msg, wParam, lParam );

    Tempest::Window* w = 0;
    std::unordered_map<WindowsAPI::Window*, Tempest::Window*>::iterator i
        = wndWx.find( (WindowsAPI::Window*)hWnd );

    if( i!= wndWx.end() )
      w = i->second;

    switch( msg ) {
      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg) );
        //w->mouseDownEvent(e);
        AbstractSystemAPI::mkMouseEvent(w, e, 0);
        }
        break;

      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg) );
        //w->mouseUpEvent(e);
        AbstractSystemAPI::mkMouseEvent(w, e, 1);
        }
        break;

      case WM_MOUSEMOVE: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      Event::ButtonNone );
        AbstractSystemAPI::mkMouseEvent(w, e, 2);
        }
        break;

      case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
        }
        break;

        ////////////////////////////////////////
      case WM_SIZE:{
          RECT rectWindow;
          GetWindowRect( HWND(hWnd), &rectWindow);
          int ww = rectWindow.right-rectWindow.left;
          int hw = rectWindow.bottom-rectWindow.left;

          if( w && !(w->w()==ww && w->h()==hw) ){
            w->resize( LOWORD(lParam), HIWORD(lParam) );
            }
          }
        break;

      case WM_CLOSE:
      case WM_DESTROY: {
        PostQuitMessage(0);
        }
        break;

      default: {
        return DefWindowProc( hWnd, msg, wParam, lParam );
        }
      }

    return 0;
    }

#endif
