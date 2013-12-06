#include "windowsapi.h"

#ifdef __WIN32__
#include <windows.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>

#include <iostream>

#include <IL/il.h>
#include "core/wrappers/atomic.h"

using namespace Tempest;

static std::unordered_map<WindowsAPI::Window*, Tempest::Window*> wndWx;

static LRESULT CALLBACK WindowProc( HWND   hWnd,
                                    UINT   msg,
                                    WPARAM wParam,
                                    LPARAM lParam );

WindowsAPI::WindowsAPI() {
  TranslateKeyPair k[] = {
    { VK_LEFT,   Event::K_Left   },
    { VK_RIGHT,  Event::K_Right  },
    { VK_UP,     Event::K_Up     },
    { VK_DOWN,   Event::K_Down   },

    { VK_ESCAPE, Event::K_ESCAPE },
    { VK_BACK,   Event::K_Back   },
    { VK_DELETE, Event::K_Delete },
    { VK_INSERT, Event::K_Insert },
    { VK_HOME,   Event::K_Home   },
    { VK_END,    Event::K_End    },
    { VK_PAUSE,  Event::K_Pause  },
    { VK_RETURN, Event::K_Return },

    { VK_F1,     Event::K_F1 },
    { 0x30,      Event::K_0  },
    { 0x41,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(24);
  }

WindowsAPI::~WindowsAPI() {
  }

bool WindowsAPI::testDisplaySettings( const DisplaySettings & s ) {
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

  return ChangeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL;
  }

bool WindowsAPI::setDisplaySettings( const DisplaySettings &s ) {
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
  if( s.fullScreen )
    flg |= CDS_FULLSCREEN;

  return 0;

  if( ChangeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL ){
    //appMode       = mode;
    //appDevModeFlg = flg;
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
  //EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &defaultMode );
  //appMode = defaultMode;

  WNDCLASSEX winClass;

  winClass.lpszClassName = L"Tempest_Window_Class";
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  winClass.lpfnWndProc   = WindowProc;
  winClass.hInstance     = GetModuleHandle(0);
  winClass.hIcon         = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hIconSm       = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  T_ASSERT_X( RegisterClassEx(&winClass), "window not initalized" );
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
    Sleep(0);
    return uMsg.wParam;
    } else {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      i->second->render();

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
      Sleep(0);
      r = uMsg.wParam;
      } else {
      for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
        if( i->second->showMode()!=Tempest::Window::Minimized )
          i->second->render();

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

std::string WindowsAPI::loadTextImpl(const char *file) {
  std::ifstream is( file, std::ifstream::binary );
  if( !is )
    return "";

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  std::string src;
  src.resize( length );
  is.read ( &src[0], length );

  if( !is )
    return "";
  is.close();

  return std::move(src);
  }

std::string WindowsAPI::loadTextImpl(const wchar_t *file) {
  HANDLE hTextFile = CreateFile( file, GENERIC_READ,
                                 0, NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwFileSize = GetFileSize(hTextFile, &dwFileSize);
  DWORD dwBytesRead;

  if( dwFileSize==DWORD(-1) )
    return std::string();

  std::string str;
  str.resize(dwFileSize);
  ReadFile(hTextFile, &str[0], dwFileSize, &dwBytesRead, NULL);
  CloseHandle(hTextFile);

  return std::move(str);
  }

std::vector<char> WindowsAPI::loadBytesImpl(const char *file) {
  std::vector<char> src;

  std::ifstream is( file, std::ifstream::binary );
  if(!is)
    return src;

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  src.resize( length );
  is.read ( &src[0], length );

  if(!is)
    return src;
  is.close();

  return std::move(src);
  }

std::vector<char> WindowsAPI::loadBytesImpl(const wchar_t *file) {
  HANDLE hTextFile = CreateFile( file, GENERIC_READ,
                                 0, NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwFileSize = GetFileSize(hTextFile, &dwFileSize);
  DWORD dwBytesRead;

  std::vector<char> str;  
  if( dwFileSize==DWORD(-1) )
    return str;

  str.resize(dwFileSize);
  ReadFile(hTextFile, &str[0], dwFileSize, &dwBytesRead, NULL);
  CloseHandle(hTextFile);

  return std::move(str);
  }

bool WindowsAPI::writeBytesImpl( const wchar_t *file,
                                 const std::vector<char>& f ) {
  HANDLE hTextFile = CreateFile( file,
                                 GENERIC_READ|GENERIC_WRITE,
                                 FILE_SHARE_READ, NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL );

  if( hTextFile==0 )
    return 0;

  DWORD dwBytesWriten;
  WriteFile(hTextFile, &f[0], f.size(), &dwBytesWriten, NULL);
  CloseHandle(hTextFile);

  return true;
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

static Tempest::KeyEvent makeKeyEvent( WPARAM k,
                                       bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k);

  if( !scut ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }

  return Tempest::KeyEvent( e );
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

    if( !w )
      return DefWindowProc( hWnd, msg, wParam, lParam );

    switch( msg ) {
      case WM_CHAR:
      {
         Tempest::KeyEvent e = Tempest::KeyEvent( uint16_t(wParam) );

         DWORD wrd[3] = {
           VK_RETURN,
           VK_BACK,
           0
           };

         if( 0 == *std::find( wrd, wrd+2, wParam) ){
           SystemAPI::emitEvent(w, e, Event::KeyDown);
           SystemAPI::emitEvent(w, e, Event::KeyUp);
           }
      }
      break;

      case WM_KEYDOWN:
      {
         Tempest::KeyEvent sce = makeKeyEvent(wParam, true);
         SystemAPI::emitEvent(w, sce, Event::Shortcut);

         if( !sce.isAccepted() ){
           Tempest::KeyEvent e =  makeKeyEvent(wParam);
           if( e.key!=Tempest::KeyEvent::K_NoKey )
             SystemAPI::emitEvent(w, e, Event::KeyDown);
           }
      }
      break;

      case WM_KEYUP:
      {
         Tempest::KeyEvent e =  makeKeyEvent(wParam);
         SystemAPI::emitEvent(w, e, Event::KeyUp);
      }
      break;


      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg),
                      0,
                      0,
                      Event::MouseDown );
        SystemAPI::emitEvent(w, e, Event::MouseDown);
        }
        break;

      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg),
                      0,
                      0,
                      Event::MouseUp  );
        //w->mouseUpEvent(e);
        SystemAPI::emitEvent(w, e, Event::MouseUp);
        }
        break;

      case WM_MOUSEMOVE: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      Event::ButtonNone,
                      0,
                      0,
                      Event::MouseMove  );
        SystemAPI::emitEvent(w, e, Event::MouseMove);
        }
        break;

       case WM_MOUSEWHEEL:{
          POINT p;
          p.x = LOWORD (lParam);
          p.y = HIWORD (lParam);

          ScreenToClient(hWnd, &p);

          Tempest::MouseEvent e( p.x, p.y,
                                 Tempest::Event::ButtonNone,
                                 GET_WHEEL_DELTA_WPARAM(wParam) );
          SystemAPI::emitEvent(w, e, Event::MouseWheel);
          //w->mouseWheelEvent(e);
          }
        break;

      case WM_SIZE:{
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

            SystemAPI::setShowMode( w, smode);
            SystemAPI::sizeEvent( w, cW, cH );
            GetWindowRect( HWND(hWnd), &rectWindow );
            SystemAPI::moveEvent( w, rectWindow.left, rectWindow.top );
            }
          }
        break;

      case WM_ACTIVATEAPP:
      {
          bool a = (wParam==TRUE);
          SystemAPI::activateEvent(w,a);

          if( !a && w->isFullScreenMode() ){
            ShowWindow( hWnd, SW_MINIMIZE );
            }

          if( !a ){
            //ChangeDisplaySettings(&defaultMode, 0);
            } else {
            //ChangeDisplaySettings(&appMode, appDevModeFlg);
            }
      }
      break;

      case WM_CLOSE:{
        Tempest::CloseEvent e;
        SystemAPI::emitEvent(w, e, Event::Close);
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

#endif
