#include "atomic.h"

#include <Tempest/Application>
#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <windows.h>
#else
#include <stdatomic.h>
#endif

using namespace Tempest;

Detail::Spin::Spin() {
#ifdef TEMPEST_M_TREADS
  flag.clear();
#endif
  }

void Detail::Spin::lock() {
#ifdef TEMPEST_M_TREADS
  while (flag.test_and_set(std::memory_order_acquire))
    Tempest::Application::sleep(1);
#endif
  }

void Detail::Spin::unlock() {
#ifdef TEMPEST_M_TREADS
  flag.clear(std::memory_order_release);
#endif
  }

Detail::atomic_counter Detail::atomicInc( volatile Detail::atomic_counter &src,
                                          long add) {
#ifdef TEMPEST_M_TREADS
#  if defined(__WINDOWS__)
     return InterlockedExchangeAdd( &src, add );
#  elif defined(__OSX__) || defined(__LINUX__) || defined(__ANDROID__) || defined(__IOS__)
     return __atomic_add_fetch(&src, add, __ATOMIC_SEQ_CST)-add;
#  else
#    error "Detail::atomicInc not implemented for this platform"
#  endif
#endif

  auto old = src;
  src += add;
  return old;
  }
