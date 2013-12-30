#ifndef PROGRAMOBJECT_H
#define PROGRAMOBJECT_H

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

namespace Tempest {

class ProgramObject {
  public:
    VertexShader   vs;
    FragmentShader fs;

    bool isValid() const;
  };

}

#endif // PROGRAMOBJECT_H
