#ifndef TEXTUREHOLDER_H
#define TEXTUREHOLDER_H

#include <Tempest/AbstractHolder>

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

    virtual Tempest::Texture2d create( int w, int h,
                                        AbstractTexture::Format::Type format
                                          = Tempest::AbstractTexture::Format::RGB,
                                        TextureUsage u = TU_Undefined );

    Tempest::Texture2d load( const std::string & fname );
    Tempest::Texture2d load( const char* fname );

  protected:
    virtual void createObject( AbstractAPI::Texture*& t,
                               const std::string & fname );

    virtual void createObject(AbstractAPI::Texture*& t,
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
  private:
    TextureHolder( const TextureHolder &h );
    void operator = ( const TextureHolder& ){}

    struct Data;
    Data  *data;
  friend class Texture2d;
  };

}

#endif // TEXTUREHOLDER_H
