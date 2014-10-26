#include "log.h"

#include <Tempest/Platform>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#if defined(__WINDOWS_PHONE__) || defined(_MSC_VER)
#include <windows.h>
#endif

#include <iostream>

using namespace Tempest;

Log::Log( Mode m ):m(m){

  }

Log::~Log() {
  flush();
  }

void Log::flush() {
#ifdef __ANDROID__
  if( m==Info )
     __android_log_print(ANDROID_LOG_INFO, "", "%s", st.str().c_str());

  if( m==Error )
     __android_log_print(ANDROID_LOG_ERROR, "", "%s", st.str().c_str());

  if( m==Debug )
     __android_log_print(ANDROID_LOG_DEBUG, "", "%s", st.str().c_str());
#elif defined(__WINDOWS_PHONE__)
#if !defined(_DEBUG)
  OutputDebugStringA(st.str().c_str());
  OutputDebugStringA("\r\n");
#endif
#else
#if defined(_MSC_VER) && !defined(_NDEBUG)
  OutputDebugStringA(st.str().c_str());
  OutputDebugStringA("\r\n");
#else
  if( m==Error )
    std::cerr << st.str() << std::endl;
    else
    std::cout << st.str() << std::endl;
#endif
#endif
  }
