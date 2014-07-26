#ifndef SHADERPROGRAMHOLDER_H
#define SHADERPROGRAMHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class ShaderProgram;

class ShaderProgramHolder : public AbstractHolder< ShaderProgram,
                                                   AbstractAPI::ProgramObject > {
  public:
    typedef AbstractHolder< ShaderProgram,
                            AbstractAPI::ProgramObject > BaseType;
    ShaderProgramHolder( Device& d );
    ~ShaderProgramHolder();

    typedef AbstractShadingLang::Source Source;

    virtual ShaderProgram compile( const Source& src );
    virtual ShaderProgram load(const Source& files );
    ShaderProgram surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                                 bool &hasHalfPixelOffset );
  protected:
    virtual void deleteObject( AbstractAPI::ProgramObject* t );

    virtual void  reset( AbstractAPI::ProgramObject* t );
    virtual AbstractAPI::ProgramObject*
                  restore( AbstractAPI::ProgramObject* t );
    virtual AbstractAPI::ProgramObject*
                  copy( AbstractAPI::ProgramObject* t );
  private:
    struct Data;
    Data  *data;

  friend class ShaderProgram;
  friend class Device;
  };

}

#endif // SHADERPROGRAMHOLDER_H
