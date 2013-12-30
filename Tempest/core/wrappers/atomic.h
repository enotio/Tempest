#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef TEMPEST_M_TREADS
#include <atomic>
#endif

namespace Tempest{

namespace Detail{

struct Spin {
  Spin();
  void lock();
  void unlock();

  private:
#ifdef TEMPEST_M_TREADS
    std::atomic_flag flag;
#endif
  };

template< class T >
struct GuardBase{
  GuardBase( T & s ):s(s){
    s.lock();
    }
  ~GuardBase(){
    s.unlock();
    }

  GuardBase( const GuardBase& ) = delete;
  GuardBase& operator = ( const GuardBase& ) = delete;

  T& s;
  };

typedef GuardBase<Spin> Guard;
typedef long atomic_counter;

atomic_counter atomicInc(volatile atomic_counter &src, atomic_counter add );

}

}

#endif // ATOMIC_H
