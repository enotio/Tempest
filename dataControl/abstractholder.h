#ifndef ABSTRACTTEXTUREHOLDER_H
#define ABSTRACTTEXTUREHOLDER_H

#include <Tempest/AbstractAPI>
#include "../core/wrappers/atomic.h"

#include <string>
#include <set>

namespace Tempest{

class Device;

class AbstractHolderBase{
  public:
    AbstractHolderBase( Device &d );
    virtual ~AbstractHolderBase();

    Device &device();
    Device &device() const;
  protected:
    virtual void reset()   = 0;
    virtual bool restore() = 0;

    virtual void presentEvent(){}

    Device & m_device;

  friend class Device;
  };

template< class Data, class APIDescriptor >
class AbstractHolder : public AbstractHolderBase {
  protected:
    AbstractHolder( Device & d ):AbstractHolderBase(d){
      references.reserve(4096);
      freed.reserve(4096);
      }

    ~AbstractHolder(){
      for( size_t i=0; i<freed.size(); ++i )
        delete freed[i];
      }

    struct ImplManip{
      struct Ref{
        Ref( APIDescriptor* t ):data(t), count(1){}

        APIDescriptor*  data;
        int count;

        Detail::Spin spin;
        };

      Ref * newRef(){
        return dev->addRef( Ref( 0 ) );
        }

      Ref * newRef( const Ref * base ){
        return dev->copyRef( base );
        }

      void delRef( Ref * r ){
        if( r->data )
          dev->deleteObject( r->data );

        dev->delRef( r );
        }

      ImplManip( AbstractHolder * d ):dev(d){}
      ImplManip( const ImplManip& m ):dev(m.dev){}

      bool isValid() const {
        return dev!=0;
        }

      AbstractHolder& holder() const {
        return *dev;
        }

      private:
        AbstractHolder * dev;

      };

    virtual void deleteObject( APIDescriptor* t ) = 0;

    virtual void reset( APIDescriptor* ) = 0;
    virtual APIDescriptor* restore( APIDescriptor* ) = 0;
    virtual APIDescriptor* copy( APIDescriptor* ) = 0;

  public:
    ImplManip makeManip() {
      return ImplManip( this );
      }

  private:
    typedef typename ImplManip::Ref Ref;

    Ref* addRef( const Ref& re ){
      Ref *r = 0;
      if( freed.size() ){
        r = freed.back();
        *r = Ref(re);
        freed.pop_back();
        } else {
        r = new Ref(re);
        }

      references.push_back( r );
      return references.back();
      }

    Ref* copyRef( const Ref* re ){
      return addRef( Ref( copy(re->data) ) );
      }

    void delRef( Ref * re ){
      for( size_t i=0; i<references.size(); ++i ){
        size_t id = references.size()-i-1;
        if( references[id]==re ){
          references[id] = references.back();
          references.pop_back();
          //delete re;

          if( freed.size()<references.size() )
            freed.push_back( re ); else
            delete re;

          while( freed.size()>2*references.size() ){
            delete freed.back();
            freed.pop_back();
            }

          return;
          }
        }

      delete re;
      }

  protected:
    std::vector< Ref* > references, freed;

    void reset(){
      typename std::vector< Ref* >::iterator i = this->references.begin(),
          end = this->references.end();

      for( ; i!=end; ++i ){
        reset( (*i)->data );
        }
      }

    bool restore(){
      typename std::vector< Ref* >::iterator i = this->references.begin(),
          end = this->references.end();

      bool ret = 1;
      for( ; i!=end; ++i ){
        (*i)->data = restore( (*i)->data );

        ret &= ((*i)->data!=0);
        }

      return true;//ret;
      }

  };

template< class Data, class APIDescriptor >
class AbstractHolderWithLoad : public AbstractHolder<Data, APIDescriptor> {
  public:
    AbstractHolderWithLoad( Device &d ):AbstractHolder<Data, APIDescriptor>(d){}

    typedef typename AbstractHolder<Data, APIDescriptor>::ImplManip ImplManip;

    virtual Data load( const std::wstring & fname ){
      Data obj( *this );

      createObject( obj, obj.data.value(), fname );
      return obj;
      }

    virtual Data load( const std::string & fname ){
      Data obj( *this );

      createObject( obj, obj.data.value(), fname );
      return obj;
      }

    virtual Data load( const char* fname ){
      return load( std::string(fname) );
      }

    using AbstractHolder<Data, APIDescriptor>::reset;
    using AbstractHolder<Data, APIDescriptor>::restore;

  protected:
    virtual void createObject( APIDescriptor*& t,
                               const std::string & fname ){
      std::wstring str;
      str.assign( fname.begin(), fname.end() );
      createObject(t, str);
      }
    virtual void createObject( APIDescriptor*& t,
                               const std::wstring & fname ) = 0;

    virtual void createObject( Data & object,
                               APIDescriptor*& t,
                               const std::string & fname ){
      std::wstring str;
      str.assign( fname.begin(), fname.end() );
      createObject(object, t, str);
      }

    virtual void createObject( Data & /*object*/,
                               APIDescriptor*& t,
                               const std::wstring & fname ){
      createObject(t, fname);
      }

  private:
    typedef typename ImplManip::Ref Ref;

  };

}

#endif // ABSTRACTTEXTUREHOLDER_H
