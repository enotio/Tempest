#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <vector>
#include <cstddef>

namespace Tempest{

template< class ... Args >
class signal;
class slot;
namespace Detail{
  class signalBase;
  }

class slot {
  protected:
    ~slot(){
      for(SigInfo& s : sig)
        (*s.del)(s.ptr,this);
      }

  private:
    struct SigInfo{
      void* ptr;
      void (*del)(void* t, void* ptr);
      };

    std::vector<SigInfo> sig;

  friend class Detail::signalBase;

  template< class ... Args >
  friend class Tempest::signal;
  };

namespace Detail{

class signalBase : public slot {
  protected:
    struct IEmit{
      virtual ~IEmit(){}

      virtual bool  isEq( void *t, char *f, size_t s ) = 0;
      virtual bool  isEq( void *f ) = 0;
      virtual slot* toSlot() = 0;
      };

  friend class Tempest::slot;
  };
}

/**
 * \addtogroup Core
 */
//! signal class, for signals and slots system.
template< class ... Args >
class signal : Detail::signalBase {
  public:
    ~signal();

    inline void exec( Args ... args ){
      for( size_t i=0; i<v.size(); ++i ){
        IEmit * e = reinterpret_cast<IEmit*>(v[i].impl);
        e->exec(args...);
        }
      }

    void operator()( Args ... args ){
      for( size_t i=0; i<v.size(); ++i ){
        IEmit * e = reinterpret_cast<IEmit*>(v[i].impl);
        e->exec(args...);
        }
      }

    template< class ... FuncArgs >
    void bind( signal<FuncArgs...> & t ){
      bind( t, &signal<FuncArgs...>::operator() );
      }

    template< class ... FuncArgs >
    void ubind( signal<FuncArgs...> & t ){
      ubind( t, &signal<FuncArgs...>::operator() );
      }

    template< class T, class Ret, class TBase, class ... FuncArgs >
    void bind( T &t, Ret (TBase::*f)( FuncArgs... ) ){
      v.push_back( Impl() );
      new (v.back().impl) Emit<T, Ret, TBase, FuncArgs...>(t,f);
      reg(&t);
      }

    template< class T, class Ret, class TBase, class ... FuncArgs >
    void bind( T *t, Ret (TBase::*f)( FuncArgs... ) ){
      bind(*t, f);
      }

    template< class Ret, class ... FuncArgs >
    void bind( Ret (&f)( FuncArgs... ) ){
      v.push_back( Impl() );
      new (v.back().impl) EmitFunc<Ret, FuncArgs...>(f);
      }

    template< class T, class TBase >
    void ubind( T &t, void (TBase::*f)( Args... ) ){
      for( size_t i=0; i<v.size(); ){
        IEmit * e = reinterpret_cast<IEmit*>(v[i].impl);
        char * x = (char*)(void*)(&f);

        char ch[ sizeof(f) ];
        std::copy( x, x+sizeof(f), ch );

        if( e->isEq( &t, ch, sizeof(f) ) ){
          v[i] = v.back();
          v.pop_back();
          unreg(&t);
          } else {
          ++i;
          }
        }      
      }

    template< class T, class TBase >
    void ubind( T *t, void (TBase::*f)( Args... ) ){
      ubind(*t,f);
      }

    size_t bindsCount() const{
      return v.size();
      }

    void removeBinds(){
      v.clear();
      }

  private:
    void reg(slot* x);
    void reg(void*  ){}

    void unreg(slot* x);
    void unreg(void*  ){}

    static void eraseBinds(void* s, void* this_ptr){
      signal* sg = (signal*)s;
      for( size_t i=0; i<sg->v.size(); ){
        IEmit * e = reinterpret_cast<IEmit*>(sg->v[i].impl);

        if( e->toSlot()==this_ptr ){
          sg->v[i] = sg->v.back();
          sg->v.pop_back();
          } else {
          ++i;
          }
        }
      }

    struct IEmit : Detail::signalBase::IEmit {
      virtual void exec( Args& ... args )   = 0;
      };

    template< class T, class Ret, class TBase, class ... FuncArgs >
    struct Emit : public IEmit {
      Emit( T & t, Ret (TBase::*f)( FuncArgs... ) ):obj(&t), func(f) {
        }

      void exec( Args& ... args ){
        (*obj.*func)( args... );
        }

      bool isEq( void *t, char *f, size_t s ){
        char * x = (char*)(void*)(&func);

        if( s!=sizeof(func) )
          return false;

        for( size_t i=0; i<s; ++i )
          if( x[i]!=f[i] )
            return false;

        return obj == t;
        }

      bool isEq( void * ){return 0;}

      slot* toSlot(){
        return implIsSlot(obj);
        }

      static slot* implIsSlot(Tempest::slot* s){
        return s;
        }

      static slot* implIsSlot(void*){
        return nullptr;
        }

      T * obj;
      Ret (TBase::*func)( FuncArgs... );
      };

    template< class Ret, class ... FuncArgs >
    struct EmitFunc : public IEmit {
      EmitFunc( Ret (&f)( FuncArgs... ) ): func(f) {}

      void exec( Args& ... args ){
        func( args... );
        }

      bool isEq( void *t, char *f, size_t s ){
        (void)t;
        (void)f;
        (void)s;
        return 0;
        }

      bool isEq( void * f ){
        return f==&func;
        }

      slot* toSlot(){ return nullptr; }

    Ret (&func)( FuncArgs... );
    };

    // gcc 4.4 dont undestand const_expr keyword :(
    template< size_t a, size_t b >
    struct StaticMax{
      enum {
        value = a>b ? a : b
        };
      };

    struct Impl{
      char impl[StaticMax< sizeof(void*)+20/*hack! intel italium, max fptr size = 20 bytes*/,
                           sizeof( EmitFunc<void,Args...>)>::value ];
      };

    std::vector<Impl> v;
  };

template< class ... Args >
signal<Args...>::~signal() {
  for( size_t i=0; i<v.size(); ++i){
    IEmit * e = reinterpret_cast<IEmit*>(v[i].impl);

    if( slot* s = e->toSlot() ){
      auto& sg = s->sig;
      size_t size=sg.size();
      for(size_t r=0; r<size;){
        if(sg[r].ptr==this){
          sg[r] = sg.back();
          --size;
          } else
          ++r;
        }
      sg.resize(size);
      }
    }
  }

template< class ... Args >
void signal<Args...>::reg(slot* s){
  slot::SigInfo info;
  info.ptr = this;
  info.del = &signal<Args...>::eraseBinds;
  s->sig.push_back(info);
  }

template< class ... Args >
void signal<Args...>::unreg(slot* s){
  for(size_t i=0; i<s->sig.size(); ++i){
    if(s->sig[i].ptr==this){
      s->sig[i] = s->sig.back();
      s->sig.pop_back();
      return;
      }
    }
  }

}

#endif // SIGNAL_H
