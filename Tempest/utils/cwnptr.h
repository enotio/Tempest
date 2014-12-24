#ifndef CWNPTR_H
#define CWNPTR_H

#include "../core/wrappers/atomic.h"

namespace Tempest{

namespace Detail{

template< class T, class Manip >
class Ptr {
  public:
    Ptr():r( 0 ){}
    Ptr( const Manip & m ):manip(m), r( 0 ){}

    Ptr( const Ptr& c ):manip( c.manip ){
      if( c.r )
        atomicInc( c.r->count, 1 );
      r = c.r;
      }

    ~Ptr(){
      if( !isNull() )
        delRef();
      }

    Ptr& operator = ( const Ptr& c ){
      if( this != &c ){
        if( r!=0 )
          delRef();

        r = c.r;
        manip = c.manip;

        if( !isNull() )
          atomicInc( r->count, 1 );
        }
      return *this;
      }

    T & value(){
      if( isNull() && manip.isValid() )
        makeInstance();

      return nonConstData();
      }

    const T& const_value() const {
      if( r==0 ){
        nullValue = 0;
        return nullValue;
        }

      if( isNull() && manip.isValid() )
        makeInstance();

      return r->data;
      }

    bool isNull() const{
      return r==0;
      }

    void lock(){ r->spin.lock(); }
    void unlock(){ r->spin.unlock(); }

    mutable Manip manip;
  private:
    typedef typename Manip::Ref Ref;

    void makeInstance() const {
      r = manip.newRef();
      }

    T& nonConstData(){
      Guard guard( r->spin );
      (void)guard;

      if( r==0 ){
        nullValue = 0;
        return nullValue;
        }

      if( r->count!=1 ){
        auto nr = r;
        r = manip.newRef( r );
        atomicInc( nr->count, -1 );
        }

      return r->data;
      }

    void delRef(){
      if( atomicInc( r->count, -1 )==1 )
        manip.delRef(r);
      }

    mutable Ref*  r;

    static T nullValue;
  };

template< class T, class Manip >
T Ptr<T, Manip>::nullValue = T();
}
}

#endif // CWNPTR_H
