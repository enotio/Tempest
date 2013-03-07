#ifndef LOCALVERTEXBUFFERHOLDER_H
#define LOCALVERTEXBUFFERHOLDER_H

#include <Tempest/VertexBufferHolder>
#include <vector>

namespace Tempest {

class LocalVertexBufferHolder : public Tempest::VertexBufferHolder {
  public:
    LocalVertexBufferHolder( Tempest::Device &d );
    ~LocalVertexBufferHolder();

    void setReserveSize( int sz );
  protected:
    int reserveSize;

    struct NonFreedData {
      Tempest::AbstractAPI::VertexBuffer* handle;
      int memSize;
      bool restoreIntent;
      };

    struct NonFreed{
      NonFreedData data;
      int collectIteration;
      void * userPtr;
      };

    virtual void reset();
    virtual bool restore();

    using VertexBufferHolder::reset;
    using VertexBufferHolder::restore;

    virtual void presentEvent();
    virtual void collect( std::vector< NonFreed >& nonFreed );

    void deleteObject( NonFreed& obj );
  private:
    void createObject( AbstractAPI::VertexBuffer*& t,
                       const char *src,
                       int size, int vsize );

    void deleteObject( Tempest::AbstractAPI::VertexBuffer* t );

    std::vector< NonFreed > nonFreed, dynVBOs;

    int nearPOT( int i );

    bool needToRestore;
  };

}

#endif // LOCALVERTEXBUFFERHOLDER_H
