#ifndef DIRECTX9_H
#define DIRECTX9_H

#include <Tempest/AbstractAPI>
#include <Tempest/Color>

#include <string>

namespace Tempest{

class RenderState;

class DirectX9 : public AbstractAPI {
  public:
    DirectX9();
    ~DirectX9();

    Device* createDevice( void * hwnd, const Options & opt ) const;
    void    deleteDevice( Device* d )  const;

    void clear( AbstractAPI::Device *d,
                const Color& cl, float z, unsigned stencil ) const ;

    void clear( AbstractAPI::Device *d,  const Color& cl ) const;
    void clear( AbstractAPI::Device *d,  const Color& cl, float z ) const;
    void clear( AbstractAPI::Device *d,  float z, unsigned stencil ) const;

    void clearZ( AbstractAPI::Device *d,  float z ) const;
    void clearStencil( AbstractAPI::Device *d,  unsigned stencil ) const;

    void beginPaint( AbstractAPI::Device *d ) const;
    void endPaint  ( AbstractAPI::Device *d ) const;

    void setRenderState( Device *d, const RenderState & ) const;

    void setRenderTaget( AbstractAPI::Device *d,
                         AbstractAPI::Texture     *tx, int mip,
                         int mrtSlot ) const;

    void unsetRenderTagets( AbstractAPI::Device *d,
                            int count ) const;

    void setDSSurfaceTaget(AbstractAPI::Device *d,
                            StdDSSurface *tx ) const;

    void setDSSurfaceTaget(AbstractAPI::Device *d,
                            AbstractAPI::Texture *tx) const;
    AbstractAPI::StdDSSurface* getDSSurfaceTaget( AbstractAPI::Device *d ) const;
    void retDSSurfaceTaget( AbstractAPI::Device *d,
                            AbstractAPI::StdDSSurface * s ) const;

    bool startRender( AbstractAPI::Device *d,
                      bool isLost ) const;
    bool present( AbstractAPI::Device *d ) const;
    bool reset  ( AbstractAPI::Device *d, void* hwnd,
                  const Options & opt ) const;

    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         const std::string& ) const;

    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         const Pixmap& p,
                                         bool mips,
                                         bool compress ) const;

    AbstractAPI::Texture* recreateTexture( AbstractAPI::Device * d,
                                           AbstractAPI::Texture* t,
                                           const Pixmap& p,
                                           bool mips,
                                           bool compress ) const;

    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         int w, int h, int mips,
                                         AbstractTexture::Format::Type f,
                                         TextureUsage usage  ) const;

    void deleteTexture( AbstractAPI::Device *d,
                        AbstractAPI::Texture *t) const;
/*
    AbstractAPI::RenderTagetSurface
            createRenderTaget( AbstractAPI::Device *d,
                               AbstractAPI::Texture *t,
                               int mipLevel ) const;

    void deleteRenderTaget( AbstractAPI::Device *d,
                            AbstractAPI::RenderTagetSurface t ) const;*/

    AbstractAPI::VertexBuffer*
         createVertexbuffer( AbstractAPI::Device *d,
                             size_t size, size_t elSize ) const;
    void deleteVertexBuffer( AbstractAPI::Device *d,
                             AbstractAPI::VertexBuffer* ) const;

    AbstractAPI::IndexBuffer*
         createIndexbuffer( AbstractAPI::Device *d,
                            size_t size, size_t elSize ) const;

    void deleteIndexBuffer( AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer* ) const;

    AbstractAPI::VertexDecl *
          createVertexDecl( AbstractAPI::Device *d,
                            const VertexDeclaration::Declarator &de ) const;

    void deleteVertexDecl( AbstractAPI::Device *d,
                           AbstractAPI::VertexDecl* ) const;

    void setVertexDeclaration( AbstractAPI::Device *d,
                               AbstractAPI::VertexDecl* ) const;

    void bindVertexBuffer( AbstractAPI::Device *d,
                           AbstractAPI::VertexBuffer*,
                           int vsize  ) const;
    void bindIndexBuffer( AbstractAPI::Device *d,
                          AbstractAPI::IndexBuffer* ) const;

    void* lockBuffer( AbstractAPI::Device *d,
                      AbstractAPI::VertexBuffer*,
                      unsigned offset, unsigned size) const;

    void unlockBuffer( AbstractAPI::Device *d,
                       AbstractAPI::VertexBuffer*) const;

    void* lockBuffer( AbstractAPI::Device *d,
                      AbstractAPI::IndexBuffer*,
                      unsigned offset, unsigned size) const;

    void unlockBuffer( AbstractAPI::Device *d,
                       AbstractAPI::IndexBuffer*) const;

    const AbstractShadingLang*
            createShadingLang( AbstractAPI::Device * l ) const;

    void deleteShadingLang( const AbstractShadingLang * l ) const;

    void draw( AbstractAPI::Device *d,
               AbstractAPI::PrimitiveType t,
               int firstVertex, int pCount ) const;

    void drawIndexed(  AbstractAPI::Device *d,
                       AbstractAPI::PrimitiveType t,
                       int vboOffsetIndex,
                       int minIndex,
                       int vertexCount,
                       int firstIndex,
                       int pCount ) const;
  private:
    class DirectX9Impl;
    DirectX9Impl* impl;

    struct Data;
    //Data  *data;

    DirectX9( const DirectX9& ){}
    void operator = ( const DirectX9& ){}

    void makePresentParams( void * p, void *hwnd, const Options &opt ) const;
  };

}

#endif // DIRECTX9_H
