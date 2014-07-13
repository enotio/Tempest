﻿#include "win_rt_api_binding.h"

#include <Tempest/SystemAPI>

#include <wrl.h>
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

static WinRt::MainFunction main_func=0;
static Tempest::Window *main_window = 0;

ref class App sealed : public IFrameworkView{
  private:
    static App^ inst;
  public:
    App() :
      m_windowClosed( false ),
      m_windowVisible( true ){
      inst = this;
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

      // At this point we have access to the device. 
      // We can create the device-dependent resources.
      //m_deviceResources = std::make_shared<DX::DeviceResources>();
      }

    virtual void SetWindow( Windows::UI::Core::CoreWindow^ window ){
      window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>( this, &App::OnVisibilityChanged );

      window->Closed +=
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>( this, &App::OnWindowClosed );

      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

      DisplayInformation::DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>( this, &App::OnDisplayContentsInvalidated );
      
      window->PointerPressed += 
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerPressed );
      window->PointerReleased +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerReleased );
      window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>( this, &App::OnPointerMoved );
    
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

      //m_deviceResources->SetWindow( window );
      }
    virtual void Load( Platform::String^ entryPoint ){
  
      }

    virtual void Run(){
      OutputDebugStringA( "start application" );
      static char* args[] = { "TempestAppForWindowsPhone" };
      main_func( 1, (const char**)(args) );

      while (!m_windowClosed) {
        if (m_windowVisible){
          CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessAllIfPresent );

          //m_main->Update();

          if (true)//m_main->Render())
            {
            //m_deviceResources->Present();
            }
          } else {
          CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessOneAndAllPending );
          }
        }
      }

    virtual void Uninitialize(){
    
      }

    protected:
    // Application lifecycle event handlers.
    void OnActivated( CoreApplicationView^ applicationView, IActivatedEventArgs^ args ){
      }

    void OnSuspending( Platform::Object^ sender, SuspendingEventArgs^ args ){
  
      }
    void OnResuming( Platform::Object^ sender, Platform::Object^ args ){
      int x = 0;
      }

    void OnPointerPressed( CoreWindow^window, PointerEventArgs^ event ){
      OutputDebugStringA( "onPointerPressed" );

      using namespace Tempest;
      MouseEvent e( int(event->CurrentPoint->Position.X),
                    int(event->CurrentPoint->Position.Y),
                    MouseEvent::ButtonLeft,
                    0,
                    0,
                    Event::MouseDown  );
      SystemAPI::emitEvent( main_window, e );
      }

    void OnPointerReleased( CoreWindow^window, PointerEventArgs^ event ){
      using namespace Tempest;
      MouseEvent e( int(event->CurrentPoint->Position.X),
                    int(event->CurrentPoint->Position.Y),
                    MouseEvent::ButtonLeft,
                    0,
                    0,
                    Event::MouseUp  );
      SystemAPI::emitEvent( main_window, e );
      }

    void OnPointerMoved( CoreWindow^window, PointerEventArgs^ event ){
      using namespace Tempest;
      MouseEvent e( int(event->CurrentPoint->Position.X),
                    int(event->CurrentPoint->Position.Y),
                    MouseEvent::ButtonLeft,
                    0,
                    0,
                    Event::MouseMove );
      SystemAPI::emitEvent( main_window, e );
      }

    // Window event handlers.
  #if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    void OnWindowSizeChanged( Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args );
  #endif
    void OnVisibilityChanged( CoreWindow^ sender, VisibilityChangedEventArgs^ args ){
      }

    void OnWindowClosed( CoreWindow^ sender, CoreWindowEventArgs^ args ){
  
      }

    // DisplayInformation event handlers.
  #if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    void OnDpiChanged( Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args );
    void OnOrientationChanged( Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args );
  #endif
    void OnDisplayContentsInvalidated( DisplayInformation^ sender, Object^ args ){
      }

  private:
    //std::shared_ptr<DX::DeviceResources> m_deviceResources;
    //std::unique_ptr<CubeWPMain> m_main;
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

void WinRt::nextEvent(){
  CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessOneIfPresent );
  }

void WinRt::nextEvents(){
  CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessAllIfPresent );
  }