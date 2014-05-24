#ifndef DEVICE_H
#define DEVICE_H

#include <Tempest/AbstractAPI>
#include <Tempest/AbstractShadingLang>

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/ProgramObject>

#include <Tempest/signal>

namespace Tempest{

class Color;
class Texture2d;

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
    ~Device();

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

    void generateMipMaps( Texture2d& target );

    void setRenderState( const RenderState & ) const;
    const RenderState& renderState() const;

    bool startRender();
    bool reset( const Options &opt = Options() );
    void present( AbstractAPI::SwapBehavior b = AbstractAPI::SB_BufferPreserved );

    bool hasManagedStorge() const;
    Tempest::signal<> onRestored;

    template< class S, class T >
    void setUniform( S &s, const T t[], int l, const char* name ){
      s.setUniform(name, t, l );
      }

    template< class S, class T >
    void setUniform( S &s, const T& t, const char* name ){
      s.setUniform(name, t);
      }

    template< class S, class T >
    void setUniform( S &s, const T t[], int l, const std::string& name ){
      s.setUniform(name.c_str(), t, l );
      }

    template< class S, class T >
    void setUniform( S &s, const T& t, const std::string& name ){
      s.setUniform(name.c_str(), t);
      }

    bool link( const Tempest::VertexShader   &vs,
               const Tempest::FragmentShader &fs,
               const Tempest::VertexDeclaration &decl,
               std::string& log ) {
      const AbstractShadingLang& sh = shadingLang();
      return sh.link(vs, fs, (AbstractAPI::VertexDecl*)(decl.decl->impl), log);
      }

    template< class T, class ... Args >
    void draw( const T& t, Args& ... a ){
      t.render(*this,a...);
      }

    template< class T >
    void drawPrimitive( AbstractAPI::PrimitiveType t,
                        const Tempest::VertexShader   &vs,
                        const Tempest::FragmentShader &fs,
                        const Tempest::VertexDeclaration &decl,
                        const Tempest::VertexBuffer<T> & vbo,
                        int firstVertex, int pCount ){
      assertPaint();

      if( pCount==0 ||
          decl.decl==0 ||
          !vs.isValid() ||
          !fs.isValid() )
        return;

      applyRs();
      bindShaders(vs, fs);
      bind(decl);

      const AbstractShadingLang& sh = shadingLang();
      sh.setVertexDecl( (AbstractAPI::VertexDecl*)(decl.decl->impl) );
      sh.enable();
      bind( vbo.data.const_value(), sizeof(T) );
 
      draw( t, firstVertex + vbo.m_first, pCount );

      sh.disable();
      }

    template< class T, class I >
    void drawIndexed(  AbstractAPI::PrimitiveType t,
                       const Tempest::VertexShader   &vs,
                       const Tempest::FragmentShader &fs,
                       const Tempest::VertexDeclaration &decl,
                       const Tempest::VertexBuffer<T> & vbo,
                       const Tempest::IndexBuffer<I>  & ibo,
                       int vboOffsetIndex,
                       int iboOffsetIndex,
                       int pCount ){
      assertPaint();

      if( pCount==0 ||
          decl.decl==0  ||
          !vs.isValid() ||
          !fs.isValid() )
        return;

      applyRs();
      bindShaders(vs, fs);
      bind(decl);
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

    template< class T >
    void drawPrimitive( AbstractAPI::PrimitiveType t,
                        const Tempest::ProgramObject     &m,
                        const Tempest::VertexDeclaration &decl,
                        const Tempest::VertexBuffer<T>   & vbo,
                        int firstVertex, int pCount ){
      drawPrimitive( t, m.vs, m.fs, decl, vbo, firstVertex, pCount );
      }

    template< class T, class I >
    void drawIndexed(  AbstractAPI::PrimitiveType t,
                       const Tempest::ProgramObject   &m,
                       const Tempest::VertexDeclaration &decl,
                       const Tempest::VertexBuffer<T> & vbo,
                       const Tempest::IndexBuffer<I>  & ibo,
                       int vboOffsetIndex,
                       int iboOffsetIndex,
                       int pCount ){
      drawIndexed(t, m.vs, m.fs, decl, vbo, ibo, vboOffsetIndex, iboOffsetIndex, pCount );
      }

    void drawFullScreenQuad( const Tempest::ProgramObject   & p );
    void drawFullScreenQuad( const Tempest::VertexShader   & vs,
                             const Tempest::FragmentShader & fs );

    Tempest::Size windowSize() const;
    Tempest::Size viewPortSize() const;

    std::string surfaceShader( AbstractShadingLang::ShaderType t,
                               const AbstractShadingLang::UiShaderOpt&,
                               bool& hasHalfpixOffset );
    std::string surfaceShader( AbstractShadingLang::ShaderType t,
                               const AbstractShadingLang::UiShaderOpt& opt );

    void event( const GraphicsSubsystem::Event& e ) const;
  private:
    void bind( const Tempest::VertexShader   &s );
    void bind( const Tempest::FragmentShader &s );

    void assertPaint();
    void bindShaders(const Tempest::VertexShader   &vs,
                      const Tempest::FragmentShader &fs);

    void unBind( const Tempest::VertexShader   &s );
    void unBind( const Tempest::FragmentShader &s );

    void bind( const Tempest::VertexDeclaration & d );
    void bind( AbstractAPI::VertexBuffer* b, int vsize );
    void bind( AbstractAPI::IndexBuffer* b );

    void draw( AbstractAPI::PrimitiveType t,
               int firstVertex, int pCount );
    void drawIndexedPrimitive(AbstractAPI::PrimitiveType t,
                               int vboOffsetIndex, int iboOffsetIndex,
                               int pCount);
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

    void* lockBuffer( AbstractAPI::IndexBuffer*,
                      unsigned offset, unsigned size);

    void unlockBuffer( AbstractAPI::IndexBuffer*);

    AbstractAPI::VertexDecl *
          createVertexDecl( const VertexDeclaration::Declarator &de ) const;

    void deleteVertexDecl( AbstractAPI::VertexDecl* ) const;

    inline const AbstractShadingLang& shadingLang() const {
      return *shLang;
      }

    virtual void* createShaderFromSource( AbstractAPI::ShaderType t,
                                          const std::string& src,
                                          std::string & outputLog ) const;

    AbstractAPI::VertexShader*
    createVertexShaderFromSource( const std::string& src,
                                  std::string & log ) const;
    void deleteShader( AbstractAPI::VertexShader* s ) const;

    AbstractAPI::FragmentShader*
    createFragmentShaderFromSource( const std::string& src,
                                    std::string & log  ) const;
    void deleteShader( AbstractAPI::FragmentShader* s ) const;

    AbstractShadingLang * shLang;

    Device( const Device&d ):api(d.api){}
    void operator = ( const Device& ){}

    AbstractAPI::Device* impl;
    const AbstractAPI & api;

    struct Data;
    Data *data;

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
    void applyRs() const;

  template< class Shader, class APIDescriptor, AbstractShadingLang::ShaderType >
  friend class ShaderHolder;

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
