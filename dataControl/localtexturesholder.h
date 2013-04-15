#ifndef LOCALTEXTURESHOLDER_H
#define LOCALTEXTURESHOLDER_H

#include <Tempest/TextureHolder>
#include <vector>

namespace Tempest {

class LocalTexturesHolder : public Tempest::TextureHolder {
  public:
    LocalTexturesHolder( Tempest::Device &d );
    ~LocalTexturesHolder();

  protected:
    struct NonFreedData {
      Tempest::AbstractAPI::Texture* handle;

      int  w,h;
      bool mip;
      bool compress;

      Tempest::AbstractTexture::Format::Type format;
      Tempest::TextureUsage usage;

      bool dynamic, restoreIntent;

      bool cmp( const NonFreedData& b ) const{
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
    virtual void collect( std::vector< NonFreed >& nonFreed );

    void deleteObject( NonFreed& obj );
  private:
    void createObject( Tempest::AbstractAPI::Texture*& t,
                       int w, int h, bool mips,
                       Tempest::AbstractTexture::Format::Type f,
                       Tempest::TextureUsage u );

    void createObject(AbstractAPI::Texture*& t,
                       const Pixmap & p, bool mips , bool compress);

    void deleteObject( Tempest::AbstractAPI::Texture* t );

    std::vector< NonFreed > nonFreed, dynTextures;

    bool needToRestore;
  };

}

#endif // LOCALTEXTURESHOLDER_H
