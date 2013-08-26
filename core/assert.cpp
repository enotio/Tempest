#include "assert.h"

#include <iostream>
#include <sstream>
#include <cassert>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace Tempest{
  static void h_default( const char */*file*/, int /*line*/,
                         const char */*X*/, const char */*msg*/) {
#ifdef __ANDROID__
    __android_log_assert("","","");
    assert(0);
#else
    __asm__("int $3");
#endif
    //assert(0 && "T_ASSERT failed");
    }

  static void (*h_assert)( const char *,
                           int, const char *,
                           const char *) = h_default;
  }

void Tempest::Detail::t_assert( bool a,
                                const char *file,
                                int line,
                                const char *X, const char *msg) {
  if(a)
    return;

  std::stringstream ss;
  ss << "ASSERT failure: \"" << X <<"\"";

  if( msg )
    ss << ",\t\"" << msg <<"\"\t";

  ss << "[file " << file <<", line " << line <<"]";

#ifdef __ANDROID__
  __android_log_print( ANDROID_LOG_ERROR, "Tempest", "%s", ss.str().c_str() );
#else
  std::cerr << ss.str() << std::endl;
#endif

  h_assert(file, line, X, msg);
  }

void Tempest::installAssertHandler( void (*f)( const char *,
                                               int, const char *,
                                               const char *)) {
  if( f )
    h_assert = f;
  else
    h_assert = h_default;
  }
