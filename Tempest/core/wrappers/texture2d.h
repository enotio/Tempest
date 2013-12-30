#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <Tempest/AbstractTexture>
#include <Tempest/TextureHolder>

#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/Utility>

namespace Tempest{

  class Texture2d : public AbstractTexture {
    public:
      Texture2d();
      Texture2d( AbstractHolderWithLoad< Tempest::Texture2d,
                                         AbstractAPI::Texture >& h );

      struct Sampler{
        Sampler();

        FilterType::Type minFilter, magFilter, mipFilter;
        ClampMode::Type     uClamp,    vClamp;

        void setClamping( ClampMode::Type c ){
          uClamp = c;
          vClamp = c;
          }

        bool anisotropic;
        };

      const Sampler& sampler() const;
      void setSampler( const Sampler& s );

      size_t handle() const;

      int width()  const;
      int height() const;
      Size  size() const;

      bool isEmpty() const;

      Format::Type format() const;
    private:
      Detail::Ptr<AbstractAPI::Texture*, TextureHolder::ImplManip> data;
      Sampler m_sampler;
      int w,h;

      Format::Type frm;

    friend class AbstractShadingLang;

    template< class Data, class APIDescriptor >
    friend class AbstractHolderWithLoad;

    friend class TextureHolder;
    friend class Device;
    };

  }

#endif // TEXTURE2D_H
