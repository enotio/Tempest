#ifndef VOLUMEHOLDER_H
#define VOLUMEHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/Utility>

namespace Tempest{

class Texture3d;
class Device;

class VolumeHolder : public AbstractHolder
                                < Tempest::Texture3d,
                                  AbstractAPI::Texture > {
  public:
    typedef AbstractHolder< Tempest::Texture3d,
                            AbstractAPI::Texture  > BaseType;
    VolumeHolder( Device &d );
    ~VolumeHolder();

    virtual Tempest::Texture3d load( int x, int y, int z,
                                     const char* data,
                                      AbstractTexture::Format::Type format
                                          = Tempest::AbstractTexture::Format::RGB,
                                      TextureUsage u = TU_Undefined );

    virtual Tempest::Texture3d create( int x, int y, int z,
                                        AbstractTexture::Format::Type format
                                          = Tempest::AbstractTexture::Format::RGB,
                                        TextureUsage u = TU_Undefined );

  protected:
    virtual void createObject( AbstractAPI::Texture*& t,
                               int x, int y, int z, bool mips,
                               const char *data,
                               AbstractTexture::Format::Type f,
                               TextureUsage u );

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
  private:
    VolumeHolder( const VolumeHolder &h )   = delete;
    void operator = ( const VolumeHolder& ) = delete;

    struct Data;
    Data  *data;
  friend class Texture3d;
  };

}

#endif // VOLUMEHOLDER_H
