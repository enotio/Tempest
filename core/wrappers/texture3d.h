#ifndef TEXTURE3D_H
#define TEXTURE3D_H

#include <Tempest/AbstractTexture>
#include <Tempest/VolumeHolder>

#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/Utility>

namespace Tempest{

  class Texture3d : public AbstractTexture {
    public:
      Texture3d();
      Texture3d(AbstractHolder<Texture3d, AbstractAPI::Texture> &h );

      struct Sampler{
        Sampler();

        FilterType::Type minFilter, magFilter, mipFilter;
        ClampMode::Type     uClamp,    vClamp,    wClamp;

        void setClamping( ClampMode::Type c ){
          uClamp = c;
          vClamp = c;
          wClamp = c;
          }
        };

      const Sampler& sampler() const;
      void setSampler( const Sampler& s );

      size_t handle() const;

      int   width()  const;
      int   height() const;

      int sizeX() const;
      int sizeY() const;
      int sizeZ() const;

      bool isEmpty() const;

      Format::Type format() const;
    private:
      Detail::Ptr<AbstractAPI::Texture*, VolumeHolder::ImplManip> data;
      Sampler m_sampler;
      int sx, sy, sz;

      Format::Type frm;

    friend class AbstractShadingLang;

    template< class Data, class APIDescriptor >
    friend class AbstractHolder;

    friend class VolumeHolder;
    friend class Device;
    };

  }

#endif // TEXTURE3D_H
