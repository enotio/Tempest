#include "win_rt_api_binding.h"

#include <Tempest/SystemAPI>

#include <wrl.h>
#include <ppltasks.h>
#include <Windows.Phone.ApplicationModel.h>
#include <Windows.ApplicationModel.Core.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Phone::UI::Input;

static WinRt::MainFunction main_func=0;
static Tempest::Window *main_window = 0;
static IUnknown* mainRtWindow = 0;

struct WinEvent{
  enum Type{
    EvNone,
    EvClose,
    EvPointerMoved,
    EvPointerPressed,
    EvPointerReleased,
    EvResize,
    } type;
  int data1, data2, data3;
  };


static int dpyToPixels( float dips, float dpi ) {
  static const float dipsPerInch = 96.0f;
  return int( dips * dpi / dipsPerInch + 0.5f ); // Round to nearest integer.
  }

static std::vector<WinEvent> events;

static bool isAppClosed = false;

ref class App sealed : public IFrameworkView{
  private:
    static App^ inst;

    struct Pointer {
      int x,y;
      unsigned id;
      unsigned nid;
      };
    std::vector<Pointer> pointers;

    Pointer& pointer( unsigned id ){
      for( size_t i=0; i<pointers.size(); ++i )
        if(pointers[i].id==id)
          return pointers[i];
      pointers.push_back(Pointer{0,0,id,pointers.size()});
      return pointers.back();
      }
    
    void pointerRemove( unsigned nid ){
      static const unsigned no_id = unsigned(-1);
      for( size_t i=0; i<pointers.size(); ++i )
        if(pointers[i].nid==nid){
          pointers[i].nid = no_id;
          for(size_t r=0; r<pointers.size(); ++r){
            if( pointers[r].nid!=no_id )
              return;
            }
          pointers.clear();
          return;
          }
      }

  public:
    App() :
      m_windowClosed( false ),
      m_windowVisible( true ){
      inst = this;
      events.reserve(128);
      }

    static App^ instance(){
      return inst;
      }

    // IFrameworkView Methods.
    virtual void Initialize( CoreApplicationView^ applicationView ){
      applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>( this, &App::OnActivated );

      CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>( this, &App::OnSuspending );

      CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>( this, &App::OnResuming );

      HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &App::OnBackButtonPressed);   
      }

    virtual void SetWindow( Windows::UI::Core::CoreWindow^ window ){
      mainRtWindow = reinterpret_cast<IUnknown*>(window);

      window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>( this, &App::OnVisibilityChanged );

      window->Closed +=
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>( this, &App::OnWindowClosed );

      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

      DisplayInformation::DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>( this, &App::OnDisplayContentsInvalidated );
      
      window->PointerPressed += 
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerPressed );
      // nor need - use PointerExit
      //window->PointerReleased +=
        //ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerReleased );
      window->PointerExited +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerReleased );
      window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerMoved );
    
      window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this,&App::OnWindowSizeChanged);
  
  #if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
      window->SizeChanged +=
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>( this, &App::OnWindowSizeChanged );

      currentDisplayInformation->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>( this, &App::OnDpiChanged );

      currentDisplayInformation->OrientationChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>( this, &App::OnOrientationChanged );

      // Disable all pointer visual feedback for better performance when touching.
      // This is not supported on Windows Phone applications.
      auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
      pointerVisualizationSettings->IsContactFeedbackEnabled = false;
      pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
  #endif

      }
    virtual void Load( Platform::String^ entryPoint ){
  
      }

    virtual void Run(){
      OutputDebugStringA( "start application" );
      static char* args[] = { "TempestAppForWindowsPhone" };
      main_func( 1, (const char**)(args) );
      }

    virtual void Uninitialize(){
      m_windowClosed = true;
      isAppClosed    = true;
      }

    void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args){
      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
      const float dpi = currentDisplayInformation->LogicalDpi;

      int x = dpyToPixels( args->Size.Width,  dpi );
      int y = dpyToPixels( args->Size.Height, dpi );
      WinEvent ev = {WinEvent::EvResize, x, y};
      events.push_back(ev);
      }

    protected:
    // Application lifecycle event handlers.
    void OnActivated( CoreApplicationView^ applicationView, IActivatedEventArgs^ args ){
      CoreWindow::GetForCurrentThread()->Activate();
      }


    void OnSuspending( Platform::Object^ sender, SuspendingEventArgs^ args ){
      using namespace Tempest;
      SystemAPI::activateEvent( main_window, false );
      }
    void OnResuming( Platform::Object^ sender, Platform::Object^ args ){
      using namespace Tempest;
      SystemAPI::activateEvent( main_window, true );
      }

    void OnPointerPressed( CoreWindow^window, PointerEventArgs^ event ){
      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
      const float dpi = currentDisplayInformation->LogicalDpi;
      
      Pointer& p = pointer(event->CurrentPoint->PointerId);
      p.x = dpyToPixels( event->CurrentPoint->Position.X, dpi );
      p.y = dpyToPixels( event->CurrentPoint->Position.Y, dpi );

      WinEvent ev = {WinEvent::EvPointerPressed, p.x, p.y, p.nid };
      events.push_back(ev);
      }

    void OnPointerReleased( CoreWindow^window, PointerEventArgs^ event ){
      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
      const float dpi = currentDisplayInformation->LogicalDpi;
      
      Pointer& p = pointer(event->CurrentPoint->PointerId);
      p.x = dpyToPixels( event->CurrentPoint->Position.X, dpi );
      p.y = dpyToPixels( event->CurrentPoint->Position.Y, dpi );

      WinEvent ev = {WinEvent::EvPointerReleased, p.x, p.y, p.nid };
      pointerRemove(p.nid);
      events.push_back(ev);
      }

    void OnPointerMoved( CoreWindow^ window, PointerEventArgs^ event ){
      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
      const float dpi = currentDisplayInformation->LogicalDpi;

      Pointer& p = pointer(event->CurrentPoint->PointerId);
      p.x = dpyToPixels( event->CurrentPoint->Position.X, dpi );
      p.y = dpyToPixels( event->CurrentPoint->Position.Y, dpi );

      WinEvent ev = {WinEvent::EvPointerMoved, p.x, p.y, p.nid };
      events.push_back(ev);
      }

    // Window event handlers.
  #if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    void OnWindowSizeChanged( Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args );
  #endif
    void OnVisibilityChanged( CoreWindow^ sender, VisibilityChangedEventArgs^ args ){
      //send paint event
      }

    void OnBackButtonPressed(Object^ sender, BackPressedEventArgs^ args) {
	    if(!m_windowClosed) {
        args->Handled=true;
        WinEvent ev = {WinEvent::EvClose,0,0};
        events.push_back(ev);
        } else {
        // Do nothing. Leave args->Handled set to the current value, false.
        }
      }
    void OnWindowClosed( CoreWindow^ sender, CoreWindowEventArgs^ args ){
      using namespace Tempest;
      WinEvent ev = {WinEvent::EvClose,0,0};
      events.push_back(ev);
      m_windowClosed = true;
      isAppClosed = true;
      }

    // DisplayInformation event handlers.
  #if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    void OnDpiChanged( Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args );
    void OnOrientationChanged( Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args );
  #endif
    void OnDisplayContentsInvalidated( DisplayInformation^ sender, Object^ args ){
      }

  private:
    bool m_windowClosed;
    bool m_windowVisible;
  };

App^ App::inst = nullptr;

ref class TempestApplicationSource sealed : IFrameworkViewSource{
  public:
    virtual IFrameworkView^ CreateView(){
      return ref new App();
      }
  };


int WinRt::startApplication( MainFunction func ){
  main_func = func;
  auto app = ref new TempestApplicationSource();
  CoreApplication::Run( app );
  return 0;
  }

void WinRt::setMainWidget( void* w ){
  main_window = (Tempest::Window*)w;
  }

void* WinRt::getMainRtWidget(){
  return mainRtWindow;
  }

static void processEvent( const WinEvent& e ){
  using namespace Tempest;

  if(main_window==0)
    return;

  switch (e.type)
    {
      case WinEvent::EvPointerPressed:{
        MouseEvent e( e.data1,
                      e.data2,
                      MouseEvent::ButtonLeft,
                      0,
                      e.data3,
                      Event::MouseDown  );
        SystemAPI::emitEvent( main_window, e );
        }
        break;
      case WinEvent::EvPointerReleased:{
        MouseEvent e( e.data1,
                      e.data2,
                      MouseEvent::ButtonLeft,
                      0,
                      e.data3,
                      Event::MouseUp  );
        SystemAPI::emitEvent( main_window, e );
        }
        break;
      case WinEvent::EvPointerMoved:{
        MouseEvent e( e.data1,
                      e.data2,
                      MouseEvent::ButtonLeft,
                      0,
                      e.data3,
                      Event::MouseMove  );
        SystemAPI::emitEvent( main_window, e );
        }
        break;
      case WinEvent::EvResize:{
        //SizeEvent ex(e.data1,e.data2);
        SystemAPI::sizeEvent( main_window, e.data1, e.data2 );
        //SystemAPI::emitEvent( main_window, e );
        }
        break;
      case WinEvent::EvClose:{
        CloseEvent e;
        SystemAPI::emitEvent( main_window, e );
        if(!e.isAccepted()){
          isAppClosed = true;
          }
        }
        break;
      default:
        break;
    }
  }

int WinRt::orientation() {
  return int(DisplayInformation::GetForCurrentView()->CurrentOrientation);
  }

bool WinRt::nextEvent(){
  if(events.size()==0)
    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessOneIfPresent );
  if(events.size()==0)
    return isAppClosed;
  WinEvent e = events[0];
  events.erase(events.begin());
  processEvent(e);
  return isAppClosed;
  }

bool WinRt::nextEvents(){
  CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessAllIfPresent );
  while(events.size()>0){
    WinEvent e = events[0];
    events.erase(events.begin());
    processEvent(e);
    }
  events.clear();
  return isAppClosed;
  }

void WinRt::getScreenRect( void* hwnd, int& scrW, int& scrH ){
  if (mainRtWindow){
    CoreWindow^ window = reinterpret_cast<CoreWindow^>(mainRtWindow);
    Size logicalSize = Size( window->Bounds.Width, window->Bounds.Height );

    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
    const float dpi = currentDisplayInformation->LogicalDpi;

    scrW = dpyToPixels( logicalSize.Width,  dpi );
    scrH = dpyToPixels( logicalSize.Height, dpi );
    return;
    }
  scrW = 0;
  scrH = 0;
  }

std::wstring WinRt::getAssetsFolder(){
  return Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
  }