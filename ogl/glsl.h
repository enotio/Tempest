#ifndef GLSL_H
#define GLSL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>
#include <Tempest/Uniform>

namespace Tempest {
  class VertexShader;
  class FragmentShader;

  class Matrix4x4;

  class GLSL: public AbstractShadingLang {
    public:
      GLSL(AbstractAPI::OpenGL2xDevice *dev);
      ~GLSL();

      void beginPaint() const;
      void endPaint()   const;

      void enable() const;

      void bind( const Tempest::VertexShader& ) const;
      void bind( const Tempest::FragmentShader& ) const;

      void unBind( const Tempest::VertexShader& ) const;
      void unBind( const Tempest::FragmentShader& ) const;

      void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const;

      void* context() const;

      VertexShader* createVertexShader( const std::string& fname ) const;
      VertexShader* createVertexShaderFromSource( const std::string& src ) const;
      void          deleteVertexShader( VertexShader* s ) const;

      FragmentShader* createFragmentShader( const std::string& fname ) const;
      FragmentShader* createFragmentShaderFromSource( const std::string& src ) const;
      void            deleteFragmentShader( FragmentShader* s ) const;
    private:
      struct Data;
      Data *data;

      void setDevice() const;

      void setUniform( unsigned int s,
                       const Matrix4x4& m,
                       const char* name ) const;

      void setUniform( unsigned int s,
                       const float v[],
                       int l,
                       const char* name ) const;

      void setUniform( unsigned int s,
                       const Texture2d& t,
                       const char* name,
                       int slot ) const;

      template< class T >
      void setUniforms( unsigned int s, const T & vN, int c ) const;

      struct Texture;
    };
  }


#endif // GLSL_H
