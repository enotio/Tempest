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

      int w,h,mip;
      Tempest::AbstractTexture::Format::Type format;
      Tempest::TextureUsage usage;

      bool dynamic, restoreIntent;
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
                       int w, int h, int mip,
                       Tempest::AbstractTexture::Format::Type f,
                       Tempest::TextureUsage u );

    void createObject( AbstractAPI::Texture*& t,
                       const Pixmap & p, bool mips );

    void deleteObject( Tempest::AbstractAPI::Texture* t );

    std::vector< NonFreed > nonFreed, dynTextures;

    bool needToRestore;
  };

}

#endif // LOCALTEXTURESHOLDER_H
