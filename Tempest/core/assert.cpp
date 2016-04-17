#include "assert.h"

#include <iostream>
#include <sstream>
#include <cassert>

#ifdef __ANDROID__
#include <android/log.h>
#include <Tempest/Android>
#endif
#include <Tempest/Platform>
#include <Tempest/Log>

#ifdef __WINDOWS__
#include <windows.h>
#endif

namespace Tempest{
  static void h_default( const char *file, int line,
                         const char *func,
                         const char *X, const char *msg) {
#ifdef __ANDROID__
#ifdef NDEBUG
    __android_log_assert("","","%s","");
    assert(0);
#endif   
#elif defined(_MSC_VER)
    __debugbreak();
#elif defined(__WINDOWS__)
    __asm__("int $3");
#elif defined(__APPLE__)
    __builtin_trap();
#endif
    (void)file;
    (void)line;
    (void)func;
    (void)X;
    (void)msg;
    //assert(0 && "T_ASSERT failed");
    }

  static void h_default_warn( const char *file, int line,
                              const char *func,
                              const char *X, const char *msg) {
    (void)file;
    (void)line;
    (void)func;
    (void)X;
    (void)msg;
    //assert(0 && "T_ASSERT failed");
    }

  static void (*h_assert)( const char *, int, const char *,
                           const char *,
                           const char *) = h_default;

  static void (*h_assert_warn)( const char *, int, const char *,
                                const char *,
                                const char *) = h_default_warn;

  }

void Tempest::Detail::te_assert_impl( bool a,
                                      const char *file,
                                      int line,
                                      const char* func,
                                      const char *X,
                                      const char *msg ) {
  if(a)
    return;

  if(msg)
    Log::e("ASSERT failure: \"",X,"\"",",\t\"",msg,"\"\t","[file ",file,", line ",line,", func: '",func,"']"); else
    Log::e("ASSERT failure: \"",X,"\"",                   "[file ",file,", line ",line,", func: '",func,"']");

  h_assert(file, line, func, X, msg);
  }

void Tempest::Detail::te_warning_impl( bool a,
                                       const char *file,
                                       int line,
                                       const char* func,
                                       const char *X,
                                       const char *msg ) {
  if(a)
    return;

  if(msg)
    Log::e("WARNING: \"",X,"\"",",\t\"",msg,"\"\t","[file ",file,", line ",line,", func: '",func,"']"); else
    Log::e("WARNING: \"",X,"\"",                   "[file ",file,", line ",line,", func: '",func,"']");

  h_assert_warn(file, line, func, X, msg);
  }

void Tempest::installAssertHandler( void (*f)( const char *, int,
                                               const char *,
                                               const char *,
                                               const char *)) {
  if( f )
    h_assert = f;
  else
    h_assert = h_default;
  }

void Tempest::installWarningHandler( void (*f)( const char *, int,
                                                const char *,
                                                const char *,
                                                const char *)) {
  if( f )
    h_assert_warn = f;
  else
    h_assert_warn = h_default;
  }
