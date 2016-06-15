#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifdef __MINGW32__
#include <stdalign.h>
#endif

#include <Tempest/Assert>

namespace Tempest{

template< class ... Args >
class signal;
class slot;
namespace Detail{
  class signalBase;
  }

class slot {
  protected:
    slot(){}

    slot(const slot& other){
      //sig = other.sig;
      for(const SigInfo& s : other.sig)
        (*s.reg)(s.ptr,this);
      }

    ~slot(){
      for(SigInfo& s : sig)
        (*s.del)(s.ptr,this);
      }

    slot& operator = (slot& other) {
      for(SigInfo& s : sig)
        (*s.del)(s.ptr,this);
      sig = other.sig;
      for(SigInfo& s : sig)
        (*s.reg)(s.ptr,this);
      return *this;
      }

  private:
    struct SigInfo{
      void* ptr;
      void (*del)(void* t, void* ptr);
      void (*reg)(void* t, void* ptr);
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
    signal():size(0){}
    signal(const signal& other);
    ~signal();
    signal& operator = (const signal& sg);

    inline void exec( Args ... args ){
      for(size_t i=0;i<size;++i)
        at(i)->exec(args...);
      }

    inline void operator()( Args ... args ){
      for(size_t i=0;i<size;++i)
        at(i)->exec(args...);
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
      st.add(Emit<T, Ret, TBase, FuncArgs...>(t,f));
      size++;
      reg(&t);
      }

    template< class T, class Ret, class TBase, class ... FuncArgs >
    void bind( T *t, Ret (TBase::*f)( FuncArgs... ) ){
      bind(*t, f);
      }

    template< class Ret, class ... FuncArgs >
    void bind( Ret (&f)( FuncArgs... ) ){
      st.add(EmitFunc<Ret, FuncArgs...>(f));
      size++;
      }

    template< class T, class TBase >
    void ubind( T &t, void (TBase::*f)( Args... ) ){
      char * x = (char*)(void*)(&f);
      char ch[ sizeof(f) ];
      std::copy( x, x+sizeof(f), ch );

      for(IEmit* v=st.begin(); v<st.end();)
        if( v->isEq(&t,ch,sizeof(f)) ){
          v = st.remove(v);
          size--;
          unreg(&t);
          }else
          v = v->next();
      }

    template< class T, class TBase >
    void ubind( T *t, void (TBase::*f)( Args... ) ){
      ubind(*t,f);
      }

    size_t bindsCount() const{
      return size;
      }

    void removeBinds(){
      for(IEmit* v=st.begin(); v<st.end(); v=v->next())
        if( slot* s = v->toSlot() ){
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
      size=0;
      st.clear();
      }

  private:
    void reg(slot* x);
    void reg(void*  ){}

    void unreg(slot* x);
    void unreg(void*  ){}

    static void assignBinds(void* s, void* st){
      signal* sg = (signal*)s;
      sg->reg((slot*)st);
      }

    static void eraseBinds(void* s, void* this_ptr){
      signal* sg = (signal*)s;
      for(IEmit* v=sg->st.begin(); v<sg->st.end();)
        if( v->toSlot()==this_ptr ){
          v = sg->st.remove(v);
          sg->size--;
          } else
          v = v->next();
      }

    struct IEmit : Detail::signalBase::IEmit {
      virtual void   exec( Args& ... args ) = 0;
      virtual IEmit* next()                 = 0;
      virtual size_t size() const           = 0;
      };

    template< class T, class Ret, class TBase, class ... FuncArgs >
    struct Emit : public IEmit {
      enum {
#ifdef _MSC_VER
        align = __alignof(std::max_align_t)
#else
        align = alignof(std::max_align_t)
#endif
        };

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

      IEmit* next(){
        return reinterpret_cast<IEmit*>((char*)this+size());
        }

      virtual size_t size() const {
        return ((sizeof(*this)+align-1)/align)*align;
        }

      T * obj;
      Ret (TBase::*func)( FuncArgs... );
      };

    template< class Ret, class ... FuncArgs >
    struct EmitFunc : public IEmit {
      enum {
        align = alignof(max_align_t)
        };

      EmitFunc( Ret (&f)( FuncArgs... ) ): func(f) {
        }

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

      IEmit* next(){
        return reinterpret_cast<IEmit*>((char*)this+size());
        }

      virtual size_t size() const {
        return ((sizeof(*this)+align-1)/align)*align;
        }

      Ret (&func)( FuncArgs... );
    };

    IEmit* at(size_t pos){
      for(IEmit* v=st.begin(); ; v=v->next()){
        if(pos==0) return v;
        --pos;
        }
      }

    struct Storage {
      Storage():data(nullptr),size(0){}
      ~Storage(){
        if(data)
          std::free(data);
        }

      Storage(const Storage& other){
        size = other.size;
        data = size==0 ? nullptr : (char*)malloc(other.size);
        if(!data && size>0)
          T_ASSERT_X(0,"out of memory");
        memcpy(data,other.data,size);
        }

      Storage& operator = (const Storage& other){
        data = (char*)realloc(data,other.size);
        if(!data && other.size>0)
          T_ASSERT_X(0,"out of memory");
        size = other.size;
        if(size==0)
          data=nullptr; else
          memcpy(data,other.data,size);
        return *this;
        }

      void expand(size_t sz){
        size += sz;
        char* c = (char*)realloc(data,size);
        if(!c && size>0)
          T_ASSERT_X(0,"out of memory") else
          data = c;
        }

      template<class T>
      T* add(const T& t){
        expand(t.size());
        char* ch = data+size-t.size();
        return new (ch) T(t);
        }

      IEmit* remove(IEmit* e){
        char* ep = (char*)e->next();
        char* sp = (char*)e;
        return remove(sp-data,ep-sp);
        }

      IEmit* remove(size_t at,size_t sz){
        for(size_t i=0;i<sz;++i)
          data[at+i] = data[at+i+sz];
        size -= sz;
        char* c = (char*)realloc(data,size);
        if( c )
          data = c;
        if(size==0)
          data=nullptr;
        return reinterpret_cast<IEmit*>(data+at);
        }

      void clear(){
        if(data)
          free(data);
        data=nullptr;
        size=0;
        }

      IEmit* begin(){return reinterpret_cast<IEmit*>(data);}
      void*  end()  {return data+size;}

      char*  data;
      size_t size;
      };

    Storage st;
    size_t  size;
  };

template< class ... Args >
signal<Args...>::~signal() {
  for(IEmit* v=st.begin(); v<st.end(); v=v->next())
    if( slot* s = v->toSlot() ){
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

template< class ... Args >
signal<Args...>::signal(const signal<Args...>& other):st(other.st),size(other.size){
  for(IEmit* v=st.begin(); v<st.end(); v=v->next())
    if( slot* s = v->toSlot() )
      reg(s);
  }

template< class ... Args >
signal<Args...> &signal<Args...>::operator =(const signal<Args...> &sg) {
  for(IEmit* v=st.begin(); v<st.end(); v=v->next()){
    if( slot* s = v->toSlot() ){
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

  st   = sg.st;
  size = sg.size;

  for(IEmit* v=st.begin(); v<st.end(); v=v->next())
    if( slot* s = v->toSlot() )
      reg(s);

  return *this;
  }

template< class ... Args >
void signal<Args...>::reg(slot* s){
  slot::SigInfo info;
  info.ptr = this;
  info.del = &signal<Args...>::eraseBinds;
  info.reg = &signal<Args...>::assignBinds;
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
