#ifndef ABSTRACTAPI_H
#define ABSTRACTAPI_H

#include <string>

#include <Tempest/VertexDeclaration>
#include <Tempest/AbstractTexture>
#include <Tempest/DisplaySettings>
#include <Tempest/Pixmap>
#include <Tempest/GraphicsSubsystem>

namespace Tempest{

class  Color;
class  AbstractShadingLang;
class  RenderState;
struct Size;

enum TextureUsage{
  TU_Static,
  TU_Undefined,
  TU_RenderTaget,
  TU_Dynamic
  };

class AbstractAPI: public GraphicsSubsystem {
  public:
    AbstractAPI(){}
    virtual ~AbstractAPI(){}

    struct Options{
      Options();

      bool vSync;
      DisplaySettings displaySettings;
      };

    struct Caps{
      int maxTextureSize;
      int maxRTCount;

      int maxVaryingVectors;
      int maxVaryingComponents;

      bool hasHalf2, hasHalf4;
      bool has3DTexture, hasNpotTexture;

      bool hasRedableDepth;
      bool hasNativeRGB, hasNativeRGBA;
      };

    enum PrimitiveType{
      Points        = 1,
      Lines         = 2,
      LinesStrip    = 3,
      Triangle      = 4,
      TriangleStrip = 5,
      TriangleFan   = 6,
      //Patches       = 7,

      lastPrimitiveType
      };

    static size_t primitiveCount(const size_t vert, PrimitiveType t );

    enum TextureFlag {
      TF_Inialized
      };

    enum BufferFlag{
      BF_NoFlags    = 0,
      BF_NoReadback = 1
      };

    enum BufferUsage {
      BU_Stream,
      BU_Static,
      BU_Dynamic
      };

    enum SwapBehavior {
      SB_BufferPreserved = 0,
      SB_BufferDestroyed
      };

    class DirectX11Device;
    class DirectX9Device;
    class OpenGL2xDevice;

    virtual Caps caps( Device* d ) const = 0;
    virtual std::string vendor( AbstractAPI::Device* d ) const = 0;
    virtual std::string renderer( AbstractAPI::Device* d ) const = 0;

    virtual bool testDisplaySettings( void* hwnd, const DisplaySettings& d ) const;
    virtual bool setDisplaySettings ( void* hwnd, const DisplaySettings& d ) const;

    virtual Device* allocDevice( void * hwnd, const Options & opt ) const = 0;
    virtual void    freeDevice ( Device* d )  const = 0;

    virtual Device* createDevice(void * hwnd,const Options & opt) const;
    virtual void    deleteDevice( Device* d )  const;

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

    virtual bool startRender( AbstractAPI::Device *d, void* hwnd, bool isLost ) const = 0;
    virtual bool present( AbstractAPI::Device *d, void *hwnd, SwapBehavior b      ) const = 0;
    virtual bool reset  ( AbstractAPI::Device *d, void* hwnd, const Options & opt ) const = 0;

    virtual bool isFormatSupported( AbstractAPI::Device *d, Pixmap::Format f ) const  = 0;
    virtual AbstractAPI::Texture* createTexture( AbstractAPI::Device *d,
                                                 const Pixmap& p,
                                                 bool mips,
                                                 bool compress ) const = 0;
    virtual AbstractAPI::Texture* recreateTexture( AbstractAPI::Device * d,
                                                   const Pixmap& p,
                                                   bool mips,
                                                   bool compress,
                                                   AbstractAPI::Texture* t ) const = 0;

    virtual AbstractAPI::Texture*
                 createTexture( AbstractAPI::Device *d,
                                int w, int h, bool mips,
                                AbstractTexture::Format::Type f,
                                TextureUsage usage ) const = 0;

    virtual AbstractAPI::Texture* createTexture3d( AbstractAPI::Device *d,
                                                   int x, int y, int z, bool mips,
                                                   AbstractTexture::Format::Type f,
                                                   TextureUsage usage,
                                                   const char *data  ) const  =0;

    static int mipCount( int width, int height, int depth );
    static int mipCount( int width, int height );
    static int mipCount( int width );
    virtual void generateMipmaps( AbstractAPI::Device * d,
                                  AbstractAPI::Texture* t ) const = 0;

    virtual void deleteTexture( AbstractAPI::Device *d,
                                AbstractAPI::Texture *t) const = 0;

    virtual void setTextureFlag( AbstractAPI::Device  *,
                                 AbstractAPI::Texture *,
                                 TextureFlag,
                                 bool ) const {}

    virtual AbstractAPI::VertexBuffer*
                createVertexBuffer( AbstractAPI::Device *d,
                                    size_t size, size_t elSize,
                                    BufferUsage u ) const = 0;

    virtual AbstractAPI::VertexBuffer* createVertexBuffer( AbstractAPI::Device *d,
                                                           size_t size,
                                                           size_t elSize,
                                                           const void *src,
                                                           BufferUsage u  ) const;

    virtual void deleteVertexBuffer( AbstractAPI::Device *d,
                                     AbstractAPI::VertexBuffer* ) const = 0;

    virtual AbstractAPI::IndexBuffer*
                createIndexBuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize,
                                   BufferUsage u  ) const = 0;

    virtual AbstractAPI::IndexBuffer*
                createIndexBuffer( AbstractAPI::Device *d,
                                   size_t size, size_t elSize,
                                   const void* src,
                                   BufferUsage u  ) const;

    virtual void deleteIndexBuffer( AbstractAPI::Device *d,
                                    AbstractAPI::IndexBuffer* ) const = 0;

    virtual AbstractAPI::VertexDecl *
                 createVertexDecl( AbstractAPI::Device *d,
                                   const VertexDeclaration::Declarator &de ) const = 0;

    virtual void deleteVertexDecl( AbstractAPI::Device *d,
                                   AbstractAPI::VertexDecl* ) const = 0;
    virtual void setVertexDeclaration( AbstractAPI::Device *d,
                                       AbstractAPI::VertexDecl*,
                                       size_t ) const = 0;

    virtual void* lockBuffer( AbstractAPI::Device *d,
                              AbstractAPI::VertexBuffer*,
                              unsigned offset, unsigned size) const = 0;

    virtual void unlockBuffer( AbstractAPI::Device *d,
                               AbstractAPI::VertexBuffer*) const = 0;

    virtual void updateBuffer( AbstractAPI::Device *d,
                               AbstractAPI::VertexBuffer*,
                               const void* data,
                               unsigned offset, unsigned size) const;

    virtual void* lockBuffer( AbstractAPI::Device *d,
                              AbstractAPI::IndexBuffer*,
                              unsigned offset, unsigned size) const = 0;

    virtual void unlockBuffer( AbstractAPI::Device *d,
                               AbstractAPI::IndexBuffer*) const = 0;

    virtual void updateBuffer( AbstractAPI::Device *d,
                               AbstractAPI::IndexBuffer*,
                               const void* data,
                               unsigned offset, unsigned size) const;

    virtual void bindVertexBuffer( AbstractAPI::Device *d,
                                   AbstractAPI::VertexBuffer*,
                                   int vsize  ) const  = 0;

    virtual void bindIndexBuffer( AbstractAPI::Device *d,
                                   AbstractAPI::IndexBuffer* ) const  = 0;

    virtual AbstractShadingLang*
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

    virtual Size windowSize( Device * dev ) const = 0;
    virtual bool hasManagedStorge() const{ return false; }


  protected:
    static int vertexCount(PrimitiveType t, const int pcount );
    static Device*  device;
    static uint32_t deviceRefCount;

  private:
    AbstractAPI(const AbstractAPI&) = delete;
    AbstractAPI& operator = (const AbstractAPI&) = delete;
  };

}

#endif // ABSTRACTAPI_H
