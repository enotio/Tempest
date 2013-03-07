#ifndef DEVICE_H
#define DEVICE_H

#include <Tempest/AbstractAPI>
#include <Tempest/AbstractShadingLang>

//#include <MyGL/VertexBuffer>

#include <string>

namespace Tempest{

class Color;
class Texture2d;

class VertexShader;
class FragmentShader;

class AbstractShadingLang;

template< class T >
class VertexBuffer;

template< class T >
class IndexBuffer;

class AbstractHolderBase;

class RenderTaget;

template< class T >
class Uniform;

class Device {
  public:  
    typedef AbstractAPI::Options Options;

    Device( const AbstractAPI & dx,
            void * windowHwnd );

    Device( const AbstractAPI & dx,
            const Options & opt,
            void * windowHwnd );
    ~Device();

    void clear( const Color& cl, double z, unsigned stemcil );

    void clear( const Color& cl );
    void clear( const Color& cl, double z );
    void clear( double z, unsigned stencil );

    void clearZ( double z );
    void clearStencil( unsigned stencil );

    void beginPaint();
    void endPaint  ();

    void setRenderState( const RenderState & ) const;

    void beginPaint( Texture2d & rt );
    void beginPaint( Texture2d & rt, Texture2d &depthStencil  );

    void beginPaint( Texture2d rt[], int count );
    void beginPaint( Texture2d rt[], int count, Texture2d &depthStencil );

    bool startRender();
    bool reset( const Options &opt = Options() );
    void present();

    template< class T >
    void setUniform( Tempest::VertexShader &s,
                     const T t[], int l,
                     const std::string & name ){
      shadingLang().setUniform(s, t, l, name);
      }

    template< class T >
    void setUniform( Tempest::VertexShader &s,
                     const T t[], int l,
                     const char* name ){
      shadingLang().setUniform(s, t, l, name);
      }

    template< class T >
    void setUniform( Tempest::FragmentShader &s,
                     const T t[], int l,
                     const std::string & name ){
      shadingLang().setUniform( s, t, l, name);
      }

    template< class T >
    void setUniform( Tempest::FragmentShader &s,
                     const Uniform<T> & u ){
      shadingLang().setUniform( s, u, u.sinput );
      }

    template< class T >
    void setUniform( Tempest::FragmentShader &s,
                     const T t[], int l,
                     const char* name ){
      shadingLang().setUniform( s, t, l, name);
      }

    template< class T >
    void setUniform( Tempest::VertexShader &s,
                     const T& t,
                     const std::string & name ){
      shadingLang().setUniform(s,t,name);
      }

    template< class T >
    void setUniform( Tempest::VertexShader &s,
                     const Uniform<T>& u ){
      shadingLang().setUniform( s, u, u.sinput );
      }

    template< class T >
    void setUniform( Tempest::VertexShader &s,
                     const T& t,
                     const char* name ){
      shadingLang().setUniform(s,t,name);
      }

    template< class T >
    void setUniform( Tempest::FragmentShader &s,
                     const T& t,
                     const std::string& name ){
      shadingLang().setUniform(s,t,name);
      }

    template< class T >
    void setUniform( Tempest::FragmentShader &s,
                     const T& t,
                     const char* name ){
      shadingLang().setUniform(s,t,name);
      }

    template< class T >
    void drawPrimitive( AbstractAPI::PrimitiveType t,
                        const Tempest::VertexShader   &vs,
                        const Tempest::FragmentShader &fs,
                        const Tempest::VertexDeclaration &decl,
                        const Tempest::VertexBuffer<T> & vbo,
                        int firstVertex, int pCount ){
      if( pCount==0 ||
          decl.decl==0 )
        return;

      bind(vs);
      bind(fs);
      bind(decl);
      bind( vbo.data.const_value(), sizeof(T) );

      draw( t, firstVertex + vbo.m_first, pCount );

      //unBind(vs);
      //unBind(fs);
      }

    template< class T, class I >
    void drawIndexed(  AbstractAPI::PrimitiveType t,
                       const Tempest::VertexShader   &vs,
                       const Tempest::FragmentShader &fs,
                       const Tempest::VertexDeclaration &decl,
                       const Tempest::VertexBuffer<T> & vbo,
                       const Tempest::IndexBuffer<I>  & ibo,
                       int vboOffsetIndex, int minIndex,
                       int vertexCount,
                       int firstIndex, int pCount ){
      bind(vs);
      bind(fs);
      bind(decl);
      bind( vbo.data.const_value(), sizeof(T) );
      bind( ibo.data.const_value() );

      drawIndexedPrimitive(  t,
                             vboOffsetIndex + vbo.m_first,
                             minIndex,
                             vertexCount,
                             firstIndex,
                             pCount );

      //unBind(vs);
      //unBind(fs);
      }

  private:
    void bind( const Tempest::VertexShader   &s );
    void bind( const Tempest::FragmentShader &s );

    void unBind( const Tempest::VertexShader   &s );
    void unBind( const Tempest::FragmentShader &s );

    void bind( const Tempest::VertexDeclaration & d );
    void bind( AbstractAPI::VertexBuffer* b, int vsize );
    void bind( AbstractAPI::IndexBuffer* b );

    void draw( AbstractAPI::PrimitiveType t,
               int firstVertex, int pCount );
    void drawIndexedPrimitive( AbstractAPI::PrimitiveType t,
                               int vboOffsetIndex,
                               int minIndex,
                               int vertexCount,
                               int firstIndex,
                               int pCount );

    void addRenderTaget( RenderTaget& h );
    void delRenderTaget( RenderTaget& h );

    void addHolder( AbstractHolderBase& h );
    void delHolder( AbstractHolderBase& h );

    void addVertexDeclaration( VertexDeclaration& h );
    void delVertexDeclaration( VertexDeclaration& h );

    AbstractAPI::Texture* createTexture( const Pixmap& p, bool mips = true );
    AbstractAPI::Texture* recreateTexture( AbstractAPI::Texture*, const Pixmap& p, bool mips = true );
    AbstractAPI::Texture* createTexture( int w, int h,
                                         int mips,
                                         AbstractTexture::Format::Type f,
                                         TextureUsage u );
    void deleteTexture( AbstractAPI::Texture* & t );

    AbstractAPI::VertexBuffer* createVertexbuffer( size_t size, size_t elSize );
    void deleteVertexBuffer( AbstractAPI::VertexBuffer* );

    AbstractAPI::IndexBuffer* createIndexBuffer( size_t size, size_t elSize );
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

    const AbstractShadingLang& shadingLang();

    const AbstractShadingLang * shLang;

    Device( const Device&d ):api(d.api){}
    void operator = ( const Device& ){}


    AbstractAPI::Device* impl;
    const AbstractAPI & api;

    struct Data;
    Data *data;

    void invalidateDeviceObjects();
    bool restoreDeviceObjects();

    void init( const AbstractAPI & dx,
               const Options & opt,
               void * windowHwnd );

  friend class VertexShaderHolder;
  friend class FragmentShaderHolder;
  friend class TextureHolder;
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
