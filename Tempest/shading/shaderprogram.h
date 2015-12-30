#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <Tempest/ShaderProgramHolder>
#include <Tempest/UniformDeclaration>

namespace Tempest {

class ShaderProgram {
  public:
    ShaderProgram();
    ShaderProgram( const ShaderProgram& p );
    ShaderProgram& operator = (const ShaderProgram&);
    ~ShaderProgram();

    bool isValid() const;
    const std::string& log() const;

    typedef ShaderProgramHolder::Source Source;
    template< class T >
    void setUniform( const T& u, const UniformDeclaration& d, unsigned reg ){
      if( ubo->size()<=reg )
        ubo->resize(reg+1);
      implSetUniform((*ubo)[reg],(const char*)&u,d);
      (*ubo)[reg].updated = false;
      }
  private:
    ShaderProgram(AbstractHolder< Tempest::ShaderProgram,
                                   AbstractAPI::ProgramObject >& h);    

    Detail::Ptr< AbstractAPI::ProgramObject*,
                 ShaderProgramHolder::ImplManip > data;
    std::string logv;
    std::shared_ptr< std::vector<AbstractShadingLang::UBO> > ubo;
    void implSetUniform(AbstractShadingLang::UBO &ubo,
                         const char *u,
                         const UniformDeclaration& d );

  friend class AbstractShadingLang;
  friend class ShaderProgramHolder;

  template< class Shader, class APIDescriptor, AbstractAPI::ShaderType >
  friend class ShaderHolder;
  };

}

#endif // SHADERPROGRAM_H
