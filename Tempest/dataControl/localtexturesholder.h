#ifndef LOCALTEXTURESHOLDER_H
#define LOCALTEXTURESHOLDER_H

#include <Tempest/TextureHolder>
#include <Tempest/LocalObjectPool>
#include <vector>

namespace Tempest {

class LocalTexturesHolder : public Tempest::TextureHolder {
  public:
    LocalTexturesHolder( Tempest::Device &d );
    ~LocalTexturesHolder();

    void setMaxCollectIterations(int c);
    int  maxCollectIterations() const;

    void setMaxReservedCount( int s );
    int  maxReservedCount() const;

    void pauseCollect( bool p );
    bool isCollectPaused() const;
  protected:
    struct NonFreedData {
      Tempest::AbstractAPI::Texture* handle;

      int  w,h;
      bool mip;
      bool compress;

      Tempest::AbstractTexture::Format::Type format;
      Tempest::TextureUsage usage;

      bool dynamic, restoreIntent;

      bool operator == ( const NonFreedData& b ) const{
        return w==b.w &&
               h==b.h &&
               mip==b.mip &&
               format==b.format &&
               usage==b.usage &&
               dynamic == b.dynamic &&
               compress == b.compress;
        }
      };

    struct NonFreed{
      NonFreedData data;
      int collectIteration;
      void * userPtr;
      };

    virtual void reset();
    virtual bool restore();

    using TextureHolder::reset;
    using TextureHolder::restore;

    virtual void presentEvent();
    void deleteObject( NonFreed& obj );
    bool deleteCond( NonFreed& obj );

    virtual void onMipmapsAdded( AbstractAPI::Texture* tg );
  private:
    void createObject( Tempest::AbstractAPI::Texture*& t,
                       int w, int h, bool mips,
                       Tempest::AbstractTexture::Format::Type f,
                       Tempest::TextureUsage u );

    void createObject(AbstractAPI::Texture*& t,
                       const Pixmap & p, bool mips , bool compress);

    void deleteObject( Tempest::AbstractAPI::Texture* t );

    bool validAs( const NonFreedData& x, const NonFreedData &d );

    std::vector< NonFreed >   dynTextures;
    LocalObjectPool<NonFreed> nonFreed;

    bool needToRestore;
    int  maxReserved;
    bool pcollect;
  };

}

#endif // LOCALTEXTURESHOLDER_H
