#ifndef GLSL_H
#define GLSL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {
/// \cond HIDDEN_SYMBOLS
  class GLSL: public AbstractShadingLang {
    public:
      GLSL( AbstractAPI::OpenGL2xDevice *dev );
      ~GLSL();

      void beginPaint() const;
      void endPaint()   const;

      void enable() const;
      void disable() const;

      void bind( const Tempest::ShaderProgram&  ) const;
      void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const;

      void* context() const;

      ProgramObject* createShaderFromSource( const Source &src,
                                             std::string &outputLog) const;
      void           deleteShader(ProgramObject* ) const;
      Source         surfaceShader( const UiShaderOpt &opt,
                                    bool &hasHalfPixelOffset ) const;

      std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                                 bool& hasHalfpixOffset ) const;
    private:
      struct Data;
      Data *data;

      void setUniforms(unsigned int prog, const UBO &in, int &slot) const;
      void setDevice() const;
      static const char* opt( const char* t, const char* f, bool v);

      void event(const DeleteEvent &e);
    };
/// \endcond
  }

#endif // GLSL_H
