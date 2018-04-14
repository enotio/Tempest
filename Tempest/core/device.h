#ifndef DEVICE_H
#define DEVICE_H

#include <Tempest/AbstractAPI>
#include <Tempest/AbstractShadingLang>

#include <Tempest/ShaderProgram>
#include <Tempest/Matrix4x4>

#include <Tempest/signal>

namespace Tempest{

class Color;
class Texture2d;
class UniformBuffer;

class AbstractShadingLang;

template< class T >
class VertexBuffer;

template< class T >
class IndexBuffer;

class AbstractHolderBase;

class Device {
  public:  
    typedef AbstractAPI::Options Options;

    Device( const AbstractAPI & dx,
            void * windowHwnd );

    Device( const AbstractAPI & dx,
            const Options & opt,
            void * windowHwnd );
    virtual ~Device();

    AbstractAPI::Caps caps() const;

    std::string vendor() const;
    std::string renderer() const;

    void clear(const Color& cl, float z, unsigned stemcil );

    void clear( const Color& cl );
    void clear( const Color& cl, float z );
    void clear( float z, unsigned stencil );

    void clearZ( float z );
    void clearStencil( unsigned stencil );

    bool testDisplaySettings( Window* w, const DisplaySettings& );
    bool setDisplaySettings ( const DisplaySettings& );

    void beginPaint( Texture2d & rt );
    void beginPaint( Texture2d & rt, Texture2d &depthStencil  );

    void beginPaint( Texture2d rt[], int count );
    void beginPaint( Texture2d rt[], int count, Texture2d &depthStencil );

    void beginPaint();
    void endPaint  ();

    bool readPixels(Pixmap &output, int x, int y, int w, int h);
    bool readPixels(Pixmap &output, int x, int y, int w, int h, int mrtSlot);
    void generateMipMaps( Texture2d& target );

    void setRenderState( const RenderState & ) const;
    const RenderState& renderState() const;

    bool startRender();
    bool reset( const Options &opt = Options() );
    void present( AbstractAPI::SwapBehavior b = AbstractAPI::SB_BufferPreserved );

    bool hasManagedStorge() const;
    Tempest::signal<> onRestored;

    template< class T, class ... Args >
    void draw( const T& t, Args& ... a ){
      t.render(*this,a...);
      }

    template< class T >
    void drawPrimitive( AbstractAPI::PrimitiveType        t,
                        const Tempest::ShaderProgram     &sh,
                        const Tempest::VertexDeclaration &decl,
                        const Tempest::VertexBuffer<T>   &vbo,
                        int firstVertex, int pCount ){
      assertPaint();

      if( pCount==0 ||
          decl.decl==nullptr ||
          vbo.size()==0 ||
          !sh.isValid() )
        return;

      applyRs();
      bind(sh);
      implDraw(t, decl, vbo, firstVertex, pCount);
      }

    template< class T, class I >
    void drawIndexed(  AbstractAPI::PrimitiveType        t,
                       const Tempest::ShaderProgram     &sh,
                       const Tempest::VertexDeclaration &decl,
                       const Tempest::VertexBuffer<T>   &vbo,
                       const Tempest::IndexBuffer<I>    &ibo,
                       int vboOffsetIndex,
                       int iboOffsetIndex,
                       int pCount ){
      assertPaint();

      if( pCount==0 ||
          decl.decl==nullptr  ||
          vbo.size()==0 ||
          ibo.size()==0 ||
          !sh.isValid() )
        return;

      applyRs();
      bind(sh);
      implDrawIndexed(t, decl, vbo, ibo, vboOffsetIndex, iboOffsetIndex, pCount);
      }

    void drawFullScreenQuad( const Tempest::ShaderProgram& p );

    Tempest::Size windowSize() const;
    Tempest::Size viewPortSize() const;

    AbstractShadingLang::Source surfaceShader( const AbstractShadingLang::UiShaderOpt& opt,
                                               bool& hasHalfPixelOffset );

    void event( const GraphicsSubsystem::Event& e ) const;

  protected:
    inline const AbstractShadingLang& shadingLang() const {
      return *shLang;
      }

    template< class T >
    inline void implDraw( AbstractAPI::PrimitiveType t,
                          const Tempest::VertexDeclaration &decl,
                          const Tempest::VertexBuffer<T> & vbo,
                          int vboOffsetIndex,
                          int pCount ){
      bind( decl, sizeof(T) );
      bind( vbo.data.const_value(), sizeof(T) );

      const AbstractShadingLang& sh = shadingLang();
      sh.setVertexDecl( (AbstractAPI::VertexDecl*)(decl.decl->impl) );
      sh.enable();

      draw( t, vboOffsetIndex + vbo.m_first, pCount );

      sh.disable();
      }

    template< class T, class I >
    inline void implDrawIndexed( AbstractAPI::PrimitiveType t,
                                 const Tempest::VertexDeclaration &decl,
                                 const Tempest::VertexBuffer<T> & vbo,
                                 const Tempest::IndexBuffer<I>  & ibo,
                                 int vboOffsetIndex,
                                 int iboOffsetIndex,
                                 int pCount ){
      bind( decl, sizeof(T) );
      bind( vbo.data.const_value(), sizeof(T) );
      bind( ibo.data.const_value() );

      const AbstractShadingLang& sh = shadingLang();
      sh.setVertexDecl( (AbstractAPI::VertexDecl*)(decl.decl->impl) );
      sh.enable();
      drawIndexedPrimitive( t,
                            vboOffsetIndex + vbo.m_first,
                            iboOffsetIndex + ibo.m_first,
                            pCount  );
      sh.disable();
      }

    void assertPaint();
    void applyRs() const;

    void bind( const Tempest::VertexDeclaration & d, size_t vsize );
    void bind( AbstractAPI::VertexBuffer* b, int vsize );
    void bind( AbstractAPI::IndexBuffer* b );

    void bind( const Tempest::ShaderProgram  &s );

  private:
    void draw( AbstractAPI::PrimitiveType t,
               int firstVertex, int pCount );
    void drawIndexedPrimitive( AbstractAPI::PrimitiveType t,
                               int vboOffsetIndex, int iboOffsetIndex,
                               int pCount );
    void addHolder( AbstractHolderBase& h );
    void delHolder( AbstractHolderBase& h );

    void addVertexDeclaration( VertexDeclaration& h );
    void delVertexDeclaration( VertexDeclaration& h );

    AbstractAPI::Texture* createTexture( const Pixmap& p,
                                         bool mips     = true,
                                         bool compress = true );

    AbstractAPI::Texture* recreateTexture( const Pixmap& p,
                                           bool mips,
                                           bool compress,
                                           AbstractAPI::Texture *t
                                           );

    AbstractAPI::Texture* createTexture( int w, int h,
                                         bool mips,
                                         AbstractTexture::Format::Type f,
                                         TextureUsage u );

    AbstractAPI::Texture* createTexture3d(int x, int y, int z, bool mips, const char *data,
                                           AbstractTexture::Format::Type f,
                                           TextureUsage u );

    void deleteTexture( AbstractAPI::Texture* & t );
    void setTextureFlag( AbstractAPI::Texture* t,
                         AbstractAPI::TextureFlag f,
                         bool v );

    AbstractAPI::VertexBuffer* createVertexbuffer( size_t size, size_t elSize,
                                                   AbstractAPI::BufferUsage u );
    AbstractAPI::VertexBuffer* createVertexbuffer( size_t size, size_t elSize,
                                                   const void *src,
                                                   AbstractAPI::BufferUsage u );
    void deleteVertexBuffer( AbstractAPI::VertexBuffer* );

    AbstractAPI::IndexBuffer* createIndexBuffer( size_t size, size_t elSize,
                                                 AbstractAPI::BufferUsage u );
    AbstractAPI::IndexBuffer* createIndexBuffer( size_t size, size_t elSize,
                                                 const void* src,
                                                 AbstractAPI::BufferUsage u );
    void deleteIndexBuffer( AbstractAPI::IndexBuffer* );

    void* lockBuffer( AbstractAPI::VertexBuffer*,
                      unsigned offset, unsigned size);

    void unlockBuffer( AbstractAPI::VertexBuffer*);

    void updateBuffer( AbstractAPI::VertexBuffer*, const void* data,
                       unsigned offset, unsigned size);

    void* lockBuffer( AbstractAPI::IndexBuffer*,
                      unsigned offset, unsigned size);

    void unlockBuffer( AbstractAPI::IndexBuffer*);

    void updateBuffer( AbstractAPI::IndexBuffer*, const void* data,
                       unsigned offset, unsigned size);

    AbstractAPI::VertexDecl *
          createVertexDecl( const VertexDeclaration::Declarator &de ) const;

    void deleteVertexDecl( AbstractAPI::VertexDecl* ) const;

    virtual AbstractAPI::ProgramObject*
        createShaderFromSource( const AbstractShadingLang::Source& src,
                                std::string & outputLog ) const;
    void deleteShader( AbstractAPI::ProgramObject*  ) const;

    void invalidateDeviceObjects();
    bool restoreDeviceObjects();

    void beginPaintImpl() const;
    void endPaintImpl() const;

    void wrapPaintBegin() const;
    void wrapPaintEnd() const;

    void init( const AbstractAPI & dx,
               const Options & opt,
               void * windowHwnd );

    void forceEndPaint() const;
    void del(void* handler,void (*del)(const AbstractAPI&, GraphicsSubsystem::Device*, AbstractShadingLang*, void*)) const;

    Device( const Device&d ) = delete;
    Device& operator = ( const Device& ) = delete;

    AbstractShadingLang* shLang;
    AbstractAPI::Device* impl;
    const AbstractAPI&   api;

    struct Data;
    Data *data;

  template< class Shader, class APIDescriptor, AbstractShadingLang::ShaderType >
  friend class ShaderHolder;
  friend class ShaderProgramHolder;

  friend class TextureHolder;
  friend class VolumeHolder;

  friend class VertexBufferHolder;
  friend class IndexBufferHolder;

  friend class VertexDeclaration;
  friend class RenderTaget;

  template< class T >
  friend class VertexBuffer;

  friend class AbstractHolderBase;
  };

}

#endif // DEVICE_H
