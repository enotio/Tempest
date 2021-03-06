#ifndef OPENGL2X_H
#define OPENGL2X_H

#include <Tempest/Color>
#include "openglbase.h"

namespace Tempest{

namespace Detail{
  struct ImplDeviceBase;
  }

class Opengl2x : public OpenGLBase {
  public:
    Opengl2x();
    ~Opengl2x();

    Caps caps(Device *d) const;

    Device* createDevice(void * hwnd,const Options & opt) const;
    void    deleteDevice( Device* d )  const;

    Device* allocDevice( void * hwnd, const Options & opt ) const;
    void    freeDevice( Device* d )  const;

    void clear( AbstractAPI::Device *d,
                const Color& cl, float z, unsigned stencil ) const ;

    void clear( AbstractAPI::Device *d,  const Color& cl ) const;
    void clear( AbstractAPI::Device *d,  const Color& cl, float z ) const;
    void clear( AbstractAPI::Device *d,  float z, unsigned stencil ) const;

    void clearZ( AbstractAPI::Device *d,  float z ) const;
    void clearStencil( AbstractAPI::Device *d,  unsigned stencil ) const;

    void beginPaint( AbstractAPI::Device *d ) const;
    void endPaint  ( AbstractAPI::Device *d ) const;

    bool readPixels(AbstractAPI::Device * d,Pixmap& output,int rt,int x,int y,int w,int h) const;

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

    bool startRender( AbstractAPI::Device *d,void* hwnd,bool isLost ) const;
    bool present( AbstractAPI::Device *d,void *hwnd, SwapBehavior b ) const;
    bool reset  ( AbstractAPI::Device *d, void* hwnd,
                  const Options & opt ) const;

    bool isFormatSupported( AbstractAPI::Device *d, Pixmap::Format f ) const;
    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         const std::string& ) const;

    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         const Pixmap& p,
                                         bool mips,
                                         bool compress ) const;

    AbstractAPI::Texture* recreateTexture( AbstractAPI::Device * d,
                                           const Pixmap& p,
                                           bool mips,
                                           bool compress,
                                           AbstractAPI::Texture* t ) const;

    AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                         int w, int h, bool mips,
                                         AbstractTexture::Format::Type f,
                                         TextureUsage usage  ) const;
    void generateMipmaps( AbstractAPI::Device * d,
                          AbstractAPI::Texture* t ) const;

    AbstractAPI::Texture* createTexture3d( AbstractAPI::Device *d,
                                           int x, int y, int z,
                                           bool mips,
                                           AbstractTexture::Format::Type f,
                                           TextureUsage usage,
                                           const char *data ) const;

    void deleteTexture( AbstractAPI::Device *d,
                        AbstractAPI::Texture *t) const;

    void setTextureFlag( AbstractAPI::Device  *d,
                         AbstractAPI::Texture *t,
                         TextureFlag f,
                         bool v ) const;

    AbstractAPI::VertexBuffer*
         createVertexBuffer( AbstractAPI::Device *d,
                             size_t size, size_t elSize,
                             BufferUsage u ) const;
    AbstractAPI::VertexBuffer* createVertexBuffer( AbstractAPI::Device *d,
                                                   size_t size,
                                                   size_t vsize,
                                                   const void * src,
                                                   BufferUsage u ) const;
    void deleteVertexBuffer( AbstractAPI::Device *d,
                             AbstractAPI::VertexBuffer* ) const;

    AbstractAPI::IndexBuffer*
         createIndexBuffer( AbstractAPI::Device *d,
                            size_t size, size_t elSize,
                            BufferUsage u  ) const;
    AbstractAPI::IndexBuffer *createIndexBuffer( AbstractAPI::Device *d,
                                                 size_t size,
                                                 size_t elSize,
                                                 const void *src,
                                                 BufferUsage u  ) const;

    void deleteIndexBuffer( AbstractAPI::Device *d,
                            AbstractAPI::IndexBuffer* ) const;

    AbstractAPI::VertexDecl *
          createVertexDecl( AbstractAPI::Device *d,
                            const VertexDeclaration::Declarator &de ) const;

    void deleteVertexDecl( AbstractAPI::Device *d,
                           AbstractAPI::VertexDecl* ) const;

    void setVertexDeclaration( AbstractAPI::Device *d,
                               AbstractAPI::VertexDecl*,
                               size_t vsize ) const;

    void bindVertexBuffer( AbstractAPI::Device *d,
                           AbstractAPI::VertexBuffer*,
                           size_t vsize  ) const;
    void bindIndexBuffer( AbstractAPI::Device *d,
                          AbstractAPI::IndexBuffer* ) const;

    void* lockBuffer( AbstractAPI::Device *d,
                      AbstractAPI::VertexBuffer*,
                      unsigned offset, unsigned size) const;

    void unlockBuffer( AbstractAPI::Device *d,
                       AbstractAPI::VertexBuffer*) const;

    void updateBuffer( AbstractAPI::Device *d,
                       AbstractAPI::VertexBuffer*,
                       const void* data,
                       unsigned offset, unsigned size) const;

    void* lockBuffer( AbstractAPI::Device *d,
                      AbstractAPI::IndexBuffer*,
                      unsigned offset, unsigned size) const;

    void unlockBuffer( AbstractAPI::Device *d,
                       AbstractAPI::IndexBuffer*) const;

    void updateBuffer( AbstractAPI::Device *d,
                       AbstractAPI::IndexBuffer*,
                       const void* data,
                       unsigned offset, unsigned size) const;

    AbstractShadingLang*
         createShadingLang( AbstractAPI::Device * l ) const;

    void deleteShadingLang( const AbstractShadingLang * l ) const;

    void draw( AbstractAPI::Device *d,
               AbstractAPI::PrimitiveType t,
               int firstVertex, int pCount ) const;

    void drawIndexed(AbstractAPI::Device *d,
                      AbstractAPI::PrimitiveType t,
                      int vboOffsetIndex, int iboOffsetIndex,
                      int pCount) const;


    virtual bool hasManagedStorge() const;
    virtual Size windowSize( Tempest::AbstractAPI::Device * dev ) const;

  protected:
    void loadTextureS3TC( AbstractAPI::Device *d,
                          Texture *t,
                          const Pixmap& p,
                          bool mips,
                          bool compress ) const;
    void loadTextureETC( AbstractAPI::Device *d,
                         Texture *t,
                         const Pixmap& p,
                         bool mips,
                         bool compress ) const;

    bool setDevice(  AbstractAPI::Device *d ) const;
    void setupViewport(AbstractAPI::Device *d,void* hwnd) const;

    bool setupFBO() const;
    void startTiledRender() const;
    void endTiledRender() const;

    AbstractAPI::Texture* createDepthStorage( AbstractAPI::Device *d,
                                              int w, int h,
                                              AbstractTexture::Format::Type f ) const;

    struct ImplDevice;
    mutable Detail::ImplDeviceBase * dev;
    virtual bool createContext( Detail::ImplDeviceBase*,
                                void* hwnd, const Options &opt ) const;
  private:
    void setupBuffers(char *vboOffsetIndex, bool on , bool enable, bool bind) const;
    void setupAttrPtr(const Tempest::VertexDeclaration::Declarator& vd,
                       char *vboOffsetIndex,
                       bool enable, bool on, bool bind ) const;
  };

}

#endif // OPENGL2X_H
