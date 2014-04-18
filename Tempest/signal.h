#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <vector>
#include <cstddef>

namespace Tempest{

template< class ... Args >
class signal{
  public:
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
      new (&v.back()) Emit<T, Ret, TBase, FuncArgs...>(t,f);
      }

    template< class T, class Ret, class TBase, class ... FuncArgs >
    void bind( T *t, Ret (TBase::*f)( FuncArgs... ) ){
      bind(*t, f);
      }

    template< class Ret, class ... FuncArgs >
    void bind( Ret (&f)( FuncArgs... ) ){
      v.push_back( Impl() );
      new (&v.back()) EmitFunc<Ret, FuncArgs...>(f);
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
    struct IEmit{
      virtual ~IEmit(){}

      virtual void exec( Args& ... args )   = 0;
      virtual bool isEq( void *t, char *f, size_t s ) = 0;
      virtual bool isEq( void *f ) = 0;
      };

    template< class T, class Ret, class TBase, class ... FuncArgs >
    struct Emit : public IEmit {
      Emit( T & t, Ret (TBase::*f)( FuncArgs... ) ):obj(t), func(f) {}

      void exec( Args& ... args ){
        (obj.*func)( args... );
        }

      bool isEq( void *t, char *f, size_t s ){
        char * x = (char*)(void*)(&func);

        if( s!=sizeof(func) )
          return false;

        for( size_t i=0; i<s; ++i )
          if( x[i]!=f[i] )
            return false;

        return &obj == t;
        }

      bool isEq( void * ){return 0;}

      T & obj;
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
      char impl[ StaticMax< sizeof(Emit<struct Unknown, void, struct Unknown>),
                            sizeof(EmitFunc<struct Unknown>)>::value ];
      };

    std::vector<Impl> v;
  };


}

#endif // SIGNAL_H
