#ifndef GLSL_H
#define GLSL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {
  class VertexShader;
  class FragmentShader;

  class Matrix4x4;

  class GLSL: public AbstractShadingLang {
    public:
      GLSL( AbstractAPI::OpenGL2xDevice *dev );
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

      VertexShader* createVertexShaderFromSource( const std::string& src,
                                                  std::string & log ) const;
      void          deleteVertexShader( VertexShader* s ) const;

      FragmentShader* createFragmentShaderFromSource( const std::string& src,
                                                      std::string & log  ) const;
      void            deleteFragmentShader( FragmentShader* s ) const;

      std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                                 bool& hasHalfpixOffset ) const;
    private:
      struct Data;
      Data *data;

      void setDevice() const;

      void setUniforms( unsigned int prog,
                        const ShaderInput & input,
                        bool textures ) const;

      void setUniform( unsigned int s,
                       const Matrix4x4& m,
                       const char* name,
                       void *&id ) const;

      void setUniform( unsigned int s,
                       const float v[],
                       int l,
                       const char* name,
                       void *&id ) const;

      void setUniform( unsigned int s,
                       const Texture2d& t,
                       const char* name,
                       int slot,
                       void *& id ) const;

      void setUniform( unsigned int s,
                       const Texture3d& t,
                       const char* name,
                       int slot,
                       void *& id ) const;

      template< class T >
      void setUniforms( unsigned int s, const T & vN, int c ) const;

      struct Texture;

      static const char* opt( const char* t, const char* f, bool v);

      void event(const DeleteEvent &e);
    };
  }


#endif // GLSL_H
