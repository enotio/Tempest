#ifndef ATOMIC_H
#define ATOMIC_H

namespace Tempest{

namespace Detail{

struct Spin {
  Spin();
  void lock();
  void unlock();

  private:
    short flag;
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

int atomicInc(int &src, int add );

}

}

#endif // ATOMIC_H
