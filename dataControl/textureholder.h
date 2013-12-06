#ifndef TEXTUREHOLDER_H
#define TEXTUREHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/Utility>

namespace Tempest{

class Texture2d;
class Device;
class Pixmap;

class TextureHolder : public AbstractHolderWithLoad
                                < Tempest::Texture2d,
                                  AbstractAPI::Texture > {
  public:
    typedef AbstractHolderWithLoad< Tempest::Texture2d,
                                    AbstractAPI::Texture  > BaseType;
    TextureHolder( Device &d );
    ~TextureHolder();

    using BaseType::load;

    virtual Tempest::Texture2d create( const Pixmap & p,
                                       bool mips = true,
                                       bool compress = true );

    virtual Tempest::Texture2d create( Size wh,
                                        AbstractTexture::Format::Type format
                                          = Tempest::AbstractTexture::Format::RGB,
                                        TextureUsage u = TU_RenderTaget );

    virtual Tempest::Texture2d create( int w, int h,
                                        AbstractTexture::Format::Type format
                                          = Tempest::AbstractTexture::Format::RGB,
                                        TextureUsage u = TU_RenderTaget );

    Tempest::Texture2d load( const std::wstring & fname );
    Tempest::Texture2d load( const std::string & fname );
    Tempest::Texture2d load( const char* fname );

  protected:
    virtual void createObject( AbstractAPI::Texture*& t,
                               const std::wstring & fname );

    virtual void createObject( AbstractAPI::Texture*& t,
                               int w, int h, bool mips,
                               AbstractTexture::Format::Type f,
                               TextureUsage u );

    virtual void createObject(AbstractAPI::Texture*& t,
                               const Pixmap & p, bool mips , bool compress);

    virtual void recreateObject(AbstractAPI::Texture*& t, AbstractAPI::Texture *old,
                                 const Pixmap & p, bool mips , bool compress);

    virtual void deleteObject( AbstractAPI::Texture* t );

    virtual void  reset( AbstractAPI::Texture* t );
    virtual AbstractAPI::Texture*
                  restore( AbstractAPI::Texture* t );
    virtual AbstractAPI::Texture*
                  copy( AbstractAPI::Texture* t );

    virtual void setTextureFlag( AbstractAPI::Texture* t,
                                 AbstractAPI::TextureFlag f,
                                 bool v );

    virtual bool hasCPUStorage();
    Pixmap pixmapOf( AbstractAPI::Texture* );
  private:
    TextureHolder( const TextureHolder &h )  = delete;
    void operator = ( const TextureHolder& ) = delete;

    struct Data;
    Data  *data;
  friend class Texture2d;
  };

}

#endif // TEXTUREHOLDER_H
