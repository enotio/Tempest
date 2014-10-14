#include "atomic.h"

#include <Tempest/Application>
#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <windows.h>
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
                                          Detail::atomic_counter add) {
#ifdef TEMPEST_M_TREADS
#ifdef __WINDOWS__
  //return std::atomic_fetch_add(&src, add);
  return InterlockedExchangeAdd( &src, add );
#else
#error "Detail::atomicInc not implemented for this platform"
#endif
#endif
  auto old = src;
  src += add;
  return old;
  }
