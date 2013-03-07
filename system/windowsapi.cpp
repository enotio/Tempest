#include "windowsapi.h"

#ifdef __WIN32__
#include <windows.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <unordered_map>

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
