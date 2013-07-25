#ifndef SHADER_H
#define SHADER_H

#include "shaderinput.h"

namespace Tempest{

class Shader {
  public:
    virtual ~Shader();

    void setUniform( const char* name, const Texture2d & t );
    void setUniform( const char* name, const Matrix4x4 & t );

    void setUniform( const char* name, float x );
    void setUniform( const char* name, float x, float y );
    void setUniform( const char* name, float x, float y, float z );
    void setUniform( const char* name, float x, float y, float z, float w );

    void setUniform( const char* name, const float  *xyzw, int l );
    void setUniform( const char* name, const double *xyzw, int l );
  protected:
    ShaderInput input;

  friend class Device;
  };

}

#endif // SHADER_H
