#ifndef CWNPTR_H
#define CWNPTR_H

#include "../core/wrappers/atomic.h"

namespace Tempest{

namespace Detail{

template< class T >
struct PtrManip{

  struct Ref{
    Ref( const T& t ):data(t), count(1){}

    T   data;
    int count;
    };

  Ref * newRef(){
    return new Ref( T() );
    }

  Ref * newRef( const Ref * base ){
    return new Ref( base->data );
    }

  void delRef( Ref * r ){
    delete r->data;
    delete r;
    }

  bool isValid() const {
    return true;
    }
  };

template< class T, class Manip = PtrManip<T> >
class Ptr {
  public:
    Ptr():r( 0 ){}
    Ptr( const Manip & m ):manip(m), r( 0 ){}

    Ptr( const Ptr& c ):manip( c.manip ){ // atomic
      Atomic::begin();
      r = c.r;

      if( !isNull() )
        ++r->count;
      Atomic::end();
      }

    ~Ptr(){
      if( !isNull() ){ // atomic
        Atomic::begin();
        delRef();
        Atomic::end();
        }
      }

    Ptr& operator = ( const Ptr& c ){ // atomic
      Atomic::begin();
      if( this != &c ){
        if( r!=0 )
          delRef();

        r = c.r;
        manip = c.manip;

        if( !isNull() )
          ++r->count;
        }

      Atomic::end();
      return *this;
      }

    T & value(){ // atomic
      Atomic::begin();
      if( isNull() && manip.isValid() )
        makeInstance();

      Atomic::end();
      return nonConstData();
      }

    const T& const_value() const { // atomic
      Atomic::begin();
      if( r==0 ){
        nullValue = 0;
        return nullValue;
        }

      if( isNull() && manip.isValid() )
        makeInstance();

      Atomic::end();
      return r->data;
      }

    bool isNull() const{
      return r==0;
      }

    mutable Manip manip;
  private:
    typedef typename Manip::Ref Ref;

    void makeInstance() const {
      r = manip.newRef();
      }

    T& nonConstData(){
      Atomic::begin();
      if( r==0 ){
        nullValue = 0;
        return nullValue;
        }

      if( r->count!=1 ){
        --r->count;
        r = manip.newRef( r );
        }

      Atomic::end();
      return r->data;
      }

    void delRef(){
      --r->count;

      if( r->count==0 )
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
