#include "log.h"

#ifdef __ANDROID__
#include <android/log.h>
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
#else
  std::cout << st.str() << std::endl;
#endif
  }
