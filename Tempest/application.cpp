#include "application.h"

#include <Tempest/Platform>
#include <Tempest/SystemAPI>
#include <Tempest/Android>
#include <Tempest/Event>

#include <Tempest/Timer>
#include <Tempest/Widget>
#include <Tempest/Assert>

#include <thread>

#if defined(__WINDOWS__)||defined(__WINDOWS_PHONE__)
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#if !defined(_INC_TYPES) && !defined(_LINUX_TIME_H) && !defined(_TIME_H) && !defined(__APPLE__) && !defined(_INC_TIME)
/// \cond HIDDEN_SYMBOLS
struct timespec {
  long tv_sec;
  long tv_nsec;
  };
/// \endcond

static LARGE_INTEGER getFILETIMEoffset() {
  SYSTEMTIME s;
  FILETIME f;
  LARGE_INTEGER t;

  s.wYear   = 1970;
  s.wMonth  = 1;
  s.wDay    = 1;
  s.wHour   = 0;
  s.wMinute = 0;
  s.wSecond = 0;
  s.wMilliseconds = 0;
  SystemTimeToFileTime( &s, &f );
  t.QuadPart = f.dwHighDateTime;
  t.QuadPart <<= 32;
  t.QuadPart |= f.dwLowDateTime;
  return (t);
  }

static int clock_gettime( int /*X*/, struct timespec *tv ) {
  LARGE_INTEGER       t;
  FILETIME            f;
  double                  microseconds;
  static LARGE_INTEGER    offset;
  static double           frequencyToMicroseconds;
  static int              initialized = 0;
  static BOOL             usePerformanceCounter = 0;

  if( !initialized ){
    LARGE_INTEGER performanceFrequency;
    initialized = 1;
    usePerformanceCounter = QueryPerformanceFrequency( &performanceFrequency );
    if (usePerformanceCounter) {
      QueryPerformanceCounter( &offset );
      frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
      } else {
      offset = getFILETIMEoffset();
      frequencyToMicroseconds = 10.;
      }
    }

  if (usePerformanceCounter){
    QueryPerformanceCounter( &t );
    } else {
    GetSystemTimeAsFileTime( &f );
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    }

  t.QuadPart  -= offset.QuadPart;
  microseconds = (double)t.QuadPart / frequencyToMicroseconds;
  t.QuadPart   = LONGLONG(microseconds);

  tv->tv_sec  = long(t.QuadPart / 1000000);
  tv->tv_nsec = t.QuadPart % 1000000;
  return (0);
  }
#endif

#include <time.h>

#include <iostream>

using namespace Tempest;

Application::App                                  Application::app;
Tempest::signal<Font>                             Application::fontChanged;
Tempest::signal<UiMetrics>                        Application::uiMetricsChanged;
Tempest::signal<const std::u16string,const Rect&> Application::showHint;

struct Application::Style : public Tempest::Style {
  ~Style(){
    app.style.release();
    }
  };

Application::Application() {
  app.ret  = -1;
  app.quit = false;
#ifdef __ANDROID__
  app.uiMetrics.uiScale = AndroidAPI::densityDpi();
#endif
  SystemAPI::instance().startApplication(0);
  }

Application::~Application() {
  SystemAPI::instance().endApplication();
  T_ASSERT_X( Widget::count==0, "not all widgets was destroyed");
  }

Font Application::mainFont() {
  return app.mainFont;
  }

void Application::setMainFont(const Font &f) {
  app.mainFont = f;
  fontChanged(app.mainFont);
  }

const UiMetrics &Application::uiMetrics() {
  return app.uiMetrics;
  }

void Application::setUiMetrics(const UiMetrics &u) {
  app.uiMetrics = u;
  uiMetricsChanged(u);
  }

void Application::setStyle(const Tempest::Style *stl) {
  if(app.style==nullptr)
    app.style.reset(new Application::Style());
  app.style->setParent(stl);
  }

const Tempest::Style& Application::style() {
  if(app.style==nullptr)
    app.style.reset(new Application::Style());
  return *app.style;
  }

int Application::exec() {
  execImpl(0);
  return app.ret;
  }

bool Application::isQuit() {
  return app.quit;
  }

void *Application::execImpl(void *) {
  while( !isQuit() ) {
    processEvents();
    }
  return 0;
  }

bool Application::processEvents( bool all ) {
  if( !app.quit ){
    if( all )
      app.ret = SystemAPI::instance().nextEvents(app.quit); else
      app.ret = SystemAPI::instance().nextEvent (app.quit);
    }

  processTimers();
  return app.quit;
  }

void Application::sleep(unsigned int msec) {
#if defined(__WINDOWS__)||defined(__WINDOWS_PHONE__)
  #ifdef _MSC_VER
    std::this_thread::sleep_for(std::chrono::milliseconds(msec));
  #else
    Sleep(msec);
  #endif
#else
  if( msec>=1000)
    sleep(msec/1000);

  if( msec%1000 )
    usleep( 1000*(msec%1000) );
#endif
  }

uint64_t Application::tickCount() {
#if defined(__WINDOWS_PHONE__)
  return GetTickCount64();
#elif defined(__WINDOWS__)
  return GetTickCount();
#elif defined(__APPLE__)
  static mach_timebase_info_data_t timeInfo;
  uint64_t machTime = mach_absolute_time();

  // Convert to nanoseconds - if this is the first time we've run, get the timebase.
  if( timeInfo.denom == 0 )
    mach_timebase_info(&timeInfo);

  // Convert the mach time to milliseconds
  uint64_t millis = ((machTime / 1000000) * timeInfo.numer) / timeInfo.denom;
  return millis;
#else
  timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  uint64_t t = (uint8_t)now.tv_sec;
  t *= 1000;
  t += now.tv_nsec/1000000;
  return t;
#endif
  }

void Application::exit() {
  app.quit = true;
  }

void Application::processTimers() {
  Application::App &a = app;

  for( size_t i=0; i<a.timer.size(); ++i ){
    if( app.timer[i] ){
      std::shared_ptr<Timer::Data> timpl = app.timer[i]->impl;
      uint64_t t = tickCount();

      uint64_t dt = (t-timpl->lastTimeout)/timpl->minterval;

      for( size_t r=0; app.timer[i] && r<dt && r<timpl->mrepeatCount; ++r ){
        timpl->lastTimeout = t;
        if( !timpl.unique() )
          timpl->timeout();
        }
      }
    }

  app.timer.resize( std::remove( app.timer.begin(), app.timer.end(), (void*)0 )
                    - app.timer.begin() );
  }
