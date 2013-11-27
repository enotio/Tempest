#include "atomic.h"

#ifdef __WIN32
#include <windows.h>
#include <winnt.h>
#endif

using namespace Tempest;

Detail::Spin::Spin():flag(0){
  }

void Detail::Spin::lock() {
#ifdef __WIN32
  //while( InterlockedCompareExchange16(&flag, 1, 0)!=1 ) Sleep(10);
#endif
  }

void Detail::Spin::unlock() {
  //InterlockedCompareExchange16(&flag, 0, 1);
  }

int Detail::atomicInc(int& src, int add) {
#ifdef __WIN32
  //return InterlockedExchangeAdd( &src, add );
#endif

  auto old = src;
  src += add;
  return old;
  }
