#ifndef ABSTRACTAPI_H
#define ABSTRACTAPI_H

#include <string>

#include <Tempest/VertexDeclaration>
#include <Tempest/AbstractTexture>

namespace Tempest{

class Color;
class AbstractShadingLang;
class RenderState;
class Pixmap;

enum TextureUsage{
  TU_Undefined,
  TU_RenderTaget,
  TU_Dynamic
  };

class AbstractAPI {
  public:
    virtual ~AbstractAPI(){}

    struct Options{
      Options();
      bool vSync;
      bool windowed;
      };

    enum PrimitiveType{
      Points        = 1,
      Lines         = 2,
      LinesStrip    = 3,
      Triangle      = 4,
      TriangleStrip = 5,
      TriangleFan   = 6,

      lastPrimitiveType
      };

    class Texture;
    class StdDSSurface;

    class IndexBuffer;
    class VertexBuffer;
    class VertexDecl;

    class FragmentShader;
    class VertexShader;

    // class RenderTaget;
    // class Surface;
/*
    struct RenderTagetSurface{
      RenderTaget * renderTaget;
      Surface     * surface;
      };
*/
    class Device;
    class DirectX9Device;
    class OpenGL2xDevice;


    virtual Device* createDevice( void * hwnd, const Options & opt ) const = 0;
    virtual void    deleteDevice( Device* d )  const = 0;

    virtual void clear( AbstractAPI::Device *d,
                        const Color& cl, float z, unsigned stencil ) const = 0;

    virtual void clear( AbstractAPI::Device *d,  const Color& cl )   const = 0;

    virtual void clear( AbstractAPI::Device *d,
                        const Color& cl, float z ) const = 0;
    virtual void clear( AbstractAPI::Device *d,
                        float z, unsigned stencil ) const = 0;

    virtual void clearZ( AbstractAPI::Device *d,
                         float z ) const = 0;
    virtual void clearStencil( AbstractAPI::Device *d,
                               unsigned stencil ) const = 0;

    virtual void setRenderState( AbstractAPI::Device *d,
                                 const RenderState& ) const = 0;


    virtual void beginPaint( AbstractAPI::Device *d ) const = 0;
    virtual void endPaint  ( AbstractAPI::Device *d ) const = 0;

    virtual void setDSSurfaceTaget( AbstractAPI::Device *d,
                                    AbstractAPI::Texture *tx ) const = 0;

    virtual void setDSSurfaceTaget( AbstractAPI::Device *d,
                                    AbstractAPI::StdDSSurface *tx ) const = 0;

    virtual AbstractAPI::StdDSSurface*
                 getDSSurfaceTaget( AbstractAPI::Device *d ) const = 0;
    virtual void retDSSurfaceTaget( AbstractAPI::Device *d,
                                    AbstractAPI::StdDSSurface * s ) const = 0;

    virtual void setRenderTaget( AbstractAPI::Device *d,
                                 AbstractAPI::Texture     *tx, int mip,
                                 int mrtSlot ) const = 0;

    virtual void unsetRenderTagets( AbstractAPI::Device *d,
                                    int count ) const = 0;

    virtual bool startRender( AbstractAPI::Device *d,
                              bool isLost ) const = 0;
    virtual bool present( AbstractAPI::Device *d ) const = 0;
    virtual bool reset  ( AbstractAPI::Device *d, void* hwnd,
                          const Options & opt ) const = 0;

    virtual AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                                 const Pixmap& p,
                                                 bool mips,
                                                 bool compress ) const = 0;
    virtual AbstractAPI::Texture* recreateTexture( AbstractAPI::Device * d,
                                                   AbstractAPI::Texture* t,
                                                   const Pixmap& p,
                                                   bool mips,
                                                   bool compress ) const = 0;

    virtual AbstractAPI::Texture*
                 createTexture( AbstractAPI::Device *d,
                                int w, int h, int mips,
                                AbstractTexture::Format::Type f,
                                TextureUsage usage ) const = 0;

    virtual void deleteTexture( AbstractAPI::Device *d,
                                AbstractAPI::Texture *t) const = 0;
/*
    virtual AbstractAPI::RenderTagetSurface
                  createRenderTaget( AbstractAPI::Device *d,
                                     AbstractAPI::Texture *t,
                                     int mipLevel ) const = 0;

    virtual void deleteRenderTaget( AbstractAPI::Device *d,
                                    AbstractAPI::RenderTagetSurface t) const = 0;*/

    virtual AbstractAPI::VertexBuffer*
                createVertexbuffer( AbstractAPI::Device *d,
                                    size_t size, size_t elSize ) const = 0;

    virtual void deleteVertexBuffer( AbstractAPI::Device *d,
                                     AbstractAPI::VertexBuffer* ) const = 0;

    virtual AbstractAPI::IndexBuffer*
                createIndexbuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize ) const = 0;

    virtual void deleteIndexBuffer( AbstractAPI::Device *d,
                                    AbstractAPI::IndexBuffer* ) const = 0;

    virtual AbstractAPI::VertexDecl *
                 createVertexDecl( AbstractAPI::Device *d,
                                   const VertexDeclaration::Declarator &de ) const = 0;

    virtual void deleteVertexDecl( AbstractAPI::Device *d,
                                   AbstractAPI::VertexDecl* ) const = 0;
    virtual void setVertexDeclaration( AbstractAPI::Device *d,
                                       AbstractAPI::VertexDecl* ) const = 0;

    virtual void* lockBuffer( AbstractAPI::Device *d,
                              AbstractAPI::VertexBuffer*,
                              unsigned offset, unsigned size) const = 0;

    virtual void unlockBuffer( AbstractAPI::Device *d,
                               AbstractAPI::VertexBuffer*) const = 0;

    virtual void* lockBuffer( AbstractAPI::Device *d,
                              AbstractAPI::IndexBuffer*,
                              unsigned offset, unsigned size) const = 0;

    virtual void unlockBuffer( AbstractAPI::Device *d,
                               AbstractAPI::IndexBuffer*) const = 0;

    virtual void bindVertexBuffer( AbstractAPI::Device *d,
                                   AbstractAPI::VertexBuffer*,
                                   int vsize  ) const  = 0;

    virtual void bindIndexBuffer( AbstractAPI::Device *d,
                                   AbstractAPI::IndexBuffer* ) const  = 0;

    virtual const AbstractShadingLang*
                    createShadingLang( AbstractAPI::Device * d    ) const = 0;
    virtual void deleteShadingLang( const AbstractShadingLang * l ) const = 0;



    virtual void draw( AbstractAPI::Device *d,
                       AbstractAPI::PrimitiveType t,
                       int firstVertex, int pCount ) const = 0;

    virtual void drawIndexed( AbstractAPI::Device *d,
                              AbstractAPI::PrimitiveType t,
                              int vboOffsetIndex,
                              int iboOffsetIndex,
                              int pCount ) const = 0;

    virtual bool hasManagedStorge() const{ return false; }
  };

}

#endif // ABSTRACTAPI_H
