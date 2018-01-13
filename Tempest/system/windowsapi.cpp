#include "windowsapi.h"

#ifdef __WINDOWS__
#include <windows.h>
#include <windowsx.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>
#include <Tempest/Log>

#include <iostream>
#include <array>
#include "core/wrappers/atomic.h"

using namespace Tempest;

static std::unordered_map<WindowsAPI::Window*, Tempest::Window*> wndWx;
static DEVMODE appMode;
static DWORD   appDevModeFlg = 0;

static LONG changeDisplaySettings( DEVMODE* m, DWORD w ){
  return ChangeDisplaySettings(m,w);
  }

static LRESULT CALLBACK WindowProc( HWND   hWnd,
                                    UINT   msg,
                                    WPARAM wParam,
                                    LPARAM lParam );

WindowsAPI::WindowsAPI() {
  TranslateKeyPair k[] = {
    { VK_LCONTROL, Event::K_Control },
    { VK_RCONTROL, Event::K_Control },
    { VK_CONTROL,  Event::K_Control },

    { VK_LEFT,     Event::K_Left    },
    { VK_RIGHT,    Event::K_Right   },
    { VK_UP,       Event::K_Up      },
    { VK_DOWN,     Event::K_Down    },

    { VK_ESCAPE,   Event::K_ESCAPE  },
    { VK_BACK,     Event::K_Back    },
    { VK_TAB,      Event::K_Tab     },
    { VK_SHIFT,    Event::K_Shift   },
    { VK_DELETE,   Event::K_Delete  },
    { VK_INSERT,   Event::K_Insert  },
    { VK_HOME,     Event::K_Home    },
    { VK_END,      Event::K_End     },
    { VK_PAUSE,    Event::K_Pause   },
    { VK_RETURN,   Event::K_Return  },

    { VK_F1,       Event::K_F1 },
    { 0x30,        Event::K_0  },
    { 0x41,        Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(24);
  }

WindowsAPI::~WindowsAPI() {
  }

bool WindowsAPI::testDisplaySettings( Window*, const DisplaySettings & s ) {
  DEVMODE mode;                   // Device Mode
  memset(&mode,0,sizeof(mode));
  mode.dmSize=sizeof(mode);

  mode.dmPelsWidth    = s.width;
  mode.dmPelsHeight   = s.height;
  mode.dmBitsPerPel   = s.bits;
  mode.dmFields       = DM_BITSPERPEL;

  if( s.width>=0 )
    mode.dmFields |= DM_PELSWIDTH;
  if( s.height>=0 )
    mode.dmFields |= DM_PELSHEIGHT;

  DWORD flg = CDS_TEST;
  if( s.fullScreen )
    flg |= CDS_FULLSCREEN;

  return changeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL;
  }

bool WindowsAPI::setDisplaySettings( Window* w, const DisplaySettings &s ) {
  DEVMODE mode;                   // Device Mode
  memset(&mode,0,sizeof(mode));
  mode.dmSize=sizeof(mode);

  mode.dmPelsWidth    = s.width;
  mode.dmPelsHeight   = s.height;
  mode.dmBitsPerPel   = s.bits;
  mode.dmFields       = DM_BITSPERPEL;

  if( s.width>=0 )
    mode.dmFields |= DM_PELSWIDTH;
  if( s.height>=0 )
    mode.dmFields |= DM_PELSHEIGHT;

  DWORD flg = 0;
  if( s.fullScreen ){
    flg |= CDS_FULLSCREEN;

    DWORD style   = WS_POPUP,
          exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

    SetWindowLong((HWND)w, GWL_STYLE,   style   );
    SetWindowLong((HWND)w, GWL_EXSTYLE, exStyle );

    RECT rect;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    SetWindowPos( (HWND)w, HWND_TOP,
                  0, 0,
                  s.width, s.height,
                  SWP_FRAMECHANGED );

    ShowWindow((HWND)w, SW_SHOW);
    SetForegroundWindow((HWND)w);
    SetFocus((HWND)w);
    UpdateWindow((HWND)w);
    } else {
    SetWindowLong((HWND)w, GWL_STYLE,   WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowLong((HWND)w, GWL_EXSTYLE, WS_EX_APPWINDOW);
    }

  if( changeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL ){
    appMode       = mode;
    appDevModeFlg = flg;

    if( s.fullScreen )
      SystemAPI::setShowMode( wndWx[w], Tempest::Window::FullScreen ); else
      SystemAPI::setShowMode( wndWx[w], Tempest::Window::Normal );
    return 1;
    }

  return 0;
  }

Size WindowsAPI::implScreenSize() {
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  int w = mode.dmPelsWidth;
  int h = mode.dmPelsHeight;

  return Size(w,h);
  }

void WindowsAPI::startApplication(ApplicationInitArgs *) {
  WNDCLASSEX winClass={};

  winClass.lpszClassName = L"Tempest_Window_Class";
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  winClass.lpfnWndProc   = WindowProc;
  winClass.hInstance     = GetModuleHandle(0);
  winClass.hIcon         = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hIconSm       = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = NULL;// (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  T_ASSERT_X( SUCCEEDED(RegisterClassEx(&winClass)), "window not initalized" );
  }

void WindowsAPI::endApplication() {
  UnregisterClass( L"Tempest_Window_Class", GetModuleHandle(0) );
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

int WindowsAPI::nextEvent(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));

  if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
    if( uMsg.message==WM_QUIT )
      quit = 1;

    TranslateMessage( &uMsg );
    DispatchMessage ( &uMsg );
    Sleep(0);
    return uMsg.wParam;
    } else {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      render( i->second );

    return 0;
    }
  }

int WindowsAPI::nextEvents(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));
  int r = 0;

  while( !quit ){
    if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
      if( uMsg.message==WM_QUIT )
        quit = 1;

      TranslateMessage( &uMsg );
      DispatchMessage ( &uMsg );

      if( uMsg.message==WM_QUIT )
        r = int(uMsg.wParam);
      } else {
      for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
        render( i->second );

      Sleep(1);
      return r;
      }
    }

  return r;
  }

WindowsAPI::Window *WindowsAPI::createWindow(int w, int h) {
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};

  AdjustWindowRect(&r, wflags, false);

  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              r.right-r.left,
                              r.bottom-r.top,
                              NULL, NULL,
                              GetModuleHandle(0), NULL );

  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowMaximized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags    = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  DWORD dwExStyle = WS_EX_APPWINDOW;

  wflags |= WS_CLIPSIBLINGS |	WS_CLIPCHILDREN;

  HWND hwnd = CreateWindowEx( dwExStyle,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL,
                              NULL,
                              GetModuleHandle(0), NULL );

  ShowWindow( hwnd, SW_MAXIMIZE );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowMinimized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0), NULL );

  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowFullScr() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_POPUP;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0),
                              NULL );

  ShowWindow( hwnd, SW_NORMAL );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
  }

bool WindowsAPI::setWindowTitle(Tempest::Window& w, const std::u16string &title) {
  HWND  hwnd = (HWND)handle(w);
  return SetWindowTextW(hwnd,LPWSTR(title.c_str()))==TRUE;
  }

Widget* WindowsAPI::addOverlay(WindowOverlay *ov) {
  if( wndWx.empty() ){
    delete ov;
    return 0;
    }

  Tempest::Window* w = wndWx.begin()->second;
  SystemAPI::addOverlay(w, ov);
  return ov;
  }

Point WindowsAPI::windowClientPos( SystemAPI::Window * hWnd ) {
  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
  return Point(rectWindow.left,rectWindow.top);
  }

Size WindowsAPI::windowClientRect( SystemAPI::Window * hWnd ) {
  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
  int cW = rectWindow.right  - rectWindow.left;
  int cH = rectWindow.bottom - rectWindow.top;

  return Size(cW,cH);
  }

void WindowsAPI::deleteWindow( Window *w ) {
  DestroyWindow( (HWND)w );
  wndWx.erase(w);
  }

void WindowsAPI::show(Window *hWnd) {
  Tempest::Window* w = 0;
  std::unordered_map<WindowsAPI::Window*, Tempest::Window*>::iterator i
      = wndWx.find( (WindowsAPI::Window*)hWnd );

  if( i!= wndWx.end() )
    w = i->second;

  if( !w )
    return;

  if( w->showMode()==Tempest::Window::FullScreen )
    return;

  HWND hwnd = (HWND)hWnd;

  switch( w->showMode() ){
    case Tempest::Window::Normal:
    case Tempest::Window::FullScreen:
      ShowWindow( hwnd, SW_NORMAL );
      break;

    case Tempest::Window::Minimized:
      ShowWindow( hwnd, SW_MINIMIZE );
      break;

    default:
      ShowWindow( hwnd, SW_MAXIMIZE );
      break;
    }

  UpdateWindow( hwnd );
  }

void WindowsAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};
  AdjustWindowRect(&r, wflags, false);

  LONG lStyles = GetWindowLong( (HWND)hw, GWL_STYLE );

  if( lStyles & WS_MINIMIZE )
    return;

  if( lStyles & WS_MAXIMIZE )
    return;

  MoveWindow( (HWND)hw,
              x,
              y,
              r.right-r.left,
              r.bottom-r.top,
              false );
  }

void WindowsAPI::bind( Window *w, Tempest::Window *wx ) {
  wndWx[w] = wx;
  }

WindowsAPI::CpuInfo WindowsAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  SYSTEM_INFO inf;
  GetSystemInfo(&inf);

  info.cpuCount = inf.dwNumberOfProcessors;
  return info;
  }

struct WindowsAPI::WinFile{
  HANDLE h;
  DWORD  pos, size;
  };

WindowsAPI::File *WindowsAPI::fopenImpl( const char *fname, const char *mode ) {
  return fopenImpl( toUtf16(fname).data(), mode );
  }

WindowsAPI::File *WindowsAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  DWORD gmode = 0, opMode = CREATE_ALWAYS;
  for( int i=0; mode[i]; ++i ){
    if( mode[i]=='r' ){
      gmode |= GENERIC_READ;
      opMode = OPEN_EXISTING;
      }

    if( mode[i]=='w' )
      gmode |= GENERIC_WRITE;

    if( mode[i]=='+' )
      opMode = OPEN_EXISTING;
    }

  WinFile* f = new WinFile();
  f->h = CreateFile( (wchar_t*)fname, gmode,
                     0, NULL, opMode,
                     FILE_ATTRIBUTE_NORMAL, NULL);
  if( f->h==INVALID_HANDLE_VALUE ){
    delete f;
    return 0;
    }

  f->pos  = 0;
  f->size = GetFileSize(f->h, 0);

  if( f->size==INVALID_FILE_SIZE ){
    delete f;
    return 0;
    }

  return (SystemAPI::File*)f;
  }

size_t WindowsAPI::readDataImpl(SystemAPI::File *f, char *dest, size_t count) {
  WinFile *fn = (WinFile*)f;
  DWORD dwBytesRead = count;
  DWORD cnt = (ReadFile( fn->h, dest, count, &dwBytesRead, NULL) ? dwBytesRead:0);

  fn->pos += cnt;
  return cnt;
  }

size_t WindowsAPI::peekImpl(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  WinFile *fn = (WinFile*)f;

  SetFilePointer( fn->h, skip, 0, FILE_CURRENT );
  DWORD dwBytesRead = count;
  DWORD cnt = (ReadFile( fn->h, dest, count, &dwBytesRead, NULL) ? dwBytesRead:0);

  fn->pos = SetFilePointer( fn->h, fn->pos, 0, FILE_BEGIN );
  return cnt;
  }

size_t WindowsAPI::writeDataImpl(SystemAPI::File *f, const char *data, size_t count) {
  DWORD dwBytesWriten = count;
  return WriteFile( ((WinFile*)f)->h, data, count, &dwBytesWriten, NULL)?dwBytesWriten:0;
  }

void WindowsAPI::flushImpl(SystemAPI::File *f) {
  WinFile *fn = (WinFile*)f;
  FlushFileBuffers(fn->h);
  }

size_t WindowsAPI::skipImpl(SystemAPI::File *f, size_t count) {
  WinFile *fn = (WinFile*)f;
  size_t pos = fn->pos;
  fn->pos = SetFilePointer( fn->h, count, 0, FILE_CURRENT );

  return fn->pos - pos;
  }

bool WindowsAPI::eofImpl(SystemAPI::File *f) {
  WinFile *fn = (WinFile*)f;
  return fn->pos==fn->size;
  }

size_t WindowsAPI::fsizeImpl( File *f ){
  WinFile *fn = (WinFile*)f;
  return fn->size;
  }

void WindowsAPI::fcloseImpl(SystemAPI::File *f) {
  CloseHandle( ((WinFile*)f)->h );
  delete ((WinFile*)f);
  }

static Event::MouseButton toButton( UINT msg, DWORD wParam ){
  if( msg==WM_LBUTTONDOWN ||
      msg==WM_LBUTTONUP )
    return Event::ButtonLeft;

  if( msg==WM_RBUTTONDOWN  ||
      msg==WM_RBUTTONUP)
    return Event::ButtonRight;

  if( msg==WM_MBUTTONDOWN ||
      msg==WM_MBUTTONUP )
    return Event::ButtonMid;

  if( msg==WM_XBUTTONDOWN ||
      msg==WM_XBUTTONUP ) {
    const WORD btn = GET_XBUTTON_WPARAM(wParam);
    if(btn==XBUTTON1)
      return Event::ButtonBack;
    if(btn==XBUTTON2)
      return Event::ButtonForward;
    }

  return Event::ButtonNone;
  }

static Tempest::KeyEvent makeKeyEvent( WPARAM k,
                                       bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k);

  if( !scut && 0 ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }

  return Tempest::KeyEvent( e );
  }

LRESULT CALLBACK WindowProc( HWND   hWnd,
                             UINT   msg,
                             const WPARAM wParam,
                             const LPARAM lParam ) {
    //return DefWindowProc( hWnd, msg, wParam, lParam );

    Tempest::Window* w = 0;
    std::unordered_map<WindowsAPI::Window*, Tempest::Window*>::iterator i
        = wndWx.find( (WindowsAPI::Window*)hWnd );

    if( i!= wndWx.end() )
      w = i->second;

    if( !w )
      return DefWindowProc( hWnd, msg, wParam, lParam );

    if(msg==WM_CHAR || msg==WM_KEYDOWN || msg==WM_KEYUP){
      if(wParam==0x7 || (wParam>0xe && wParam<0xf))
        return DefWindowProc( hWnd, msg, wParam, lParam );//undefined keys
      }

    switch( msg ) {
      case WM_CHAR:
      {
         Tempest::KeyEvent e = Tempest::KeyEvent( uint32_t(wParam) );

         std::array<DWORD,5> wrd = {{
           VK_RETURN,
           VK_BACK,
           VK_CONTROL,
           VK_TAB,
           0
           }};

         if( wrd.end() == std::find( wrd.begin(), wrd.end(), wParam) ){
           Tempest::KeyEvent ed( e.key, e.u16, Event::KeyDown );
           SystemAPI::emitEvent(w, ed);

           Tempest::KeyEvent eu( e.key, e.u16, Event::KeyUp );
           SystemAPI::emitEvent(w, eu);
           }
      }
      break;

      case WM_KEYDOWN:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent(wParam),
                               makeKeyEvent(wParam, true),
                               Event::KeyDown );
      }
      break;

      case WM_KEYUP:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent(wParam),
                               makeKeyEvent(wParam, true),
                               Event::KeyUp );
      }
      break;

      case WM_XBUTTONDOWN:
      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN: {
        SetCapture(hWnd);
        MouseEvent e( GET_X_LPARAM(lParam),
                      GET_Y_LPARAM(lParam),
                      toButton(msg,wParam),
                      0,
                      0,
                      Event::MouseDown );
        SystemAPI::emitEvent(w, e);
        }
        break;

      case WM_XBUTTONUP:
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        ReleaseCapture();
        MouseEvent e( GET_X_LPARAM(lParam),
                      GET_Y_LPARAM(lParam),
                      toButton(msg,wParam),
                      0,
                      0,
                      Event::MouseUp  );
        SystemAPI::emitEvent(w, e);
        }
        break;

      case WM_MOUSELEAVE:{

        }
        break;

      case WM_MOUSEMOVE: {
        MouseEvent e( GET_X_LPARAM(lParam),
                      GET_Y_LPARAM(lParam),
                      Event::ButtonNone,
                      0,
                      0,
                      Event::MouseMove  );
        SystemAPI::emitEvent(w, e);
        }
        break;

       case WM_MOUSEWHEEL:{
          POINT p;
          p.x = GET_X_LPARAM(lParam);
          p.y = GET_Y_LPARAM(lParam);

          ScreenToClient(hWnd, &p);

          Tempest::MouseEvent e( p.x, p.y,
                                 Tempest::Event::ButtonNone,
                                 GET_WHEEL_DELTA_WPARAM(wParam),
                                 0,
                                 Event::MouseWheel );
          SystemAPI::emitEvent(w, e);
          //w->mouseWheelEvent(e);
          }
        break;

      case WM_MOVING:
        if( w ){
          RECT rpos = {0,0,0,0};
          GetWindowRect( hWnd, &rpos );
          SystemAPI::moveEvent( w, rpos.left, rpos.top );
          }
        break;
      case WM_ERASEBKGND:  {
        render(w);
        return TRUE;
        }
      case WM_SIZING:{
        render(w);
        }
        break;
      case WM_SIZE:{
          RECT rpos = {0,0,0,0};
          GetWindowRect( hWnd, &rpos );

          RECT rectWindow;
          GetClientRect( HWND(hWnd), &rectWindow);
          int cW = rectWindow.right  - rectWindow.left;
          int cH = rectWindow.bottom - rectWindow.top;

          if( w ){
            int smode = int( w->showMode() );

            if( wParam==SIZE_RESTORED )
              smode = Window::Normal;

            if( wParam==SIZE_MAXIMIZED ||
                wParam==SIZE_MAXSHOW   )
              smode = Window::Maximized;

            if( wParam==SIZE_MINIMIZED )
              smode = Window::Minimized;

            if( !w->isFullScreenMode() )
              SystemAPI::setShowMode( w, smode);

            if( wParam!=SIZE_MINIMIZED ){
              SystemAPI::sizeEvent( w, cW, cH );
              //GetWindowRect( HWND(hWnd), &rectWindow );
              SystemAPI::moveEvent( w, rpos.left, rpos.top );
              }
            render(w);
            }
          }
        break;
      case WM_PAINT:
      {
        render(w);
        return DefWindowProc( hWnd, msg, wParam, lParam );
      }
      break;

      case WM_ACTIVATEAPP:
      {
          bool a = (wParam==TRUE);
          SystemAPI::activateEvent(w,a);

          WindowsAPI& api = (WindowsAPI&)SystemAPI::instance();
          api.clearPressedImpl();

          if( !a && w->isFullScreenMode() ){
            ShowWindow( hWnd, SW_MINIMIZE );
            }

          if( !a ){
            if( w->isFullScreenMode() )
              changeDisplaySettings(nullptr, 0);
            } else {
            if( w->isFullScreenMode() ){
              changeDisplaySettings(&appMode, appDevModeFlg);
              UpdateWindow((HWND)w);
              }
            }
      }
      break;

      case WM_CLOSE:{
        Tempest::CloseEvent e;
        SystemAPI::emitEvent(w, e);
        if( !e.isAccepted() )
          PostQuitMessage(0);
        }
        break;

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

void WindowsAPI::clearPressedImpl(){
  clearPressed();
  }

static
HCURSOR pixmapToCursor( const Pixmap& pinput,
                        int hotSpotX,
                        int hotSpotY ) {
  if( pinput.isEmpty() ) {
    return 0;
    }

  Pixmap pm = pinput;
  pm.setFormat( Pixmap::Format_RGBA );

  ICONINFO iconInfo;
  ZeroMemory(&iconInfo, sizeof(iconInfo));
  iconInfo.fIcon = false;

  iconInfo.xHotspot = hotSpotX;
  iconInfo.yHotspot = hotSpotY;

  HBITMAP hBitmap     = 0;
  HBITMAP hMonoBitmap = CreateBitmap( pm.width(), pm.height(), 1,1, NULL);
  iconInfo.hbmMask  = hMonoBitmap;

  {
    BITMAPV5HEADER bi;
    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size            = sizeof(BITMAPV5HEADER);
    bi.bV5Width           = pm.width();
    bi.bV5Height          = pm.height();
    bi.bV5Planes   = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.

    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000;

    HDC hdc = GetDC(NULL);

    uint8_t *lpBits;
    const uint8_t* input = pm.const_data();

    hBitmap = CreateDIBSection( hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
                                (void **)&lpBits, NULL, (DWORD)0 );

    size_t bperRow = pm.width()*4;
    for( int i=0; i<pm.height(); ++i ){
      memcpy( lpBits + bperRow*i,
              input  + bperRow*(pm.height()-i-1),
              bperRow );
      }

    size_t bsz = pm.width()*pm.height()*4;
    for( size_t i=0; i<bsz; i+=4 ){
      uint8_t a     = *(lpBits+i);
      *(lpBits+i)   = *(lpBits+i+2);
      *(lpBits+i+2) = a;
      }

    iconInfo.hbmColor = hBitmap;
    }

  HICON hicon = CreateIconIndirect(&iconInfo);

  DeleteObject(hBitmap);
  DeleteObject(hMonoBitmap);
  return (HCURSOR)hicon;
  }

void WindowsAPI::setCursor( Tempest::Window &w,
                            const Pixmap &p,
                            int hotSpotX,
                            int hotSpotY ) {
  HWND  hwnd = (HWND)handle(w);
  HCURSOR cr = pixmapToCursor(p, hotSpotX, hotSpotY);

  SetClassLongPtrW( hwnd,         // window handle
                    GCLP_HCURSOR, // change cursor
                    reinterpret_cast<LONG_PTR>(cr) );
  DestroyCursor( cr );
  }

std::string WindowsAPI::iso3Locale() {
  /*
  WCHAR loc[LOCALE_NAME_MAX_LENGTH] = {};
  int l = GetUserDefaultLocaleName(loc,LOCALE_NAME_MAX_LENGTH);
  if(l==3){
    std::string s;
    s.resize(3);
    for(int i=0; i<3; ++i )
      s[i] = loc[i];
    return s;
    }

  return "eng";*/
  const LCID id = GetUserDefaultLCID();
  static const struct{LCID id;const char* name;} loc[] = {
    {1033,"eng"},
    {1049,"rus"},
    {0,(const char*)0}
    };

  for(int i=0; loc[i].name; ++i)
    if(loc[i].id==id)
      return loc[i].name;

  return "eng";
  }

#endif
