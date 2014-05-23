#ifndef OPENGL4X_H
#define OPENGL4X_H

#include <Tempest/Opengl2x>

namespace Tempest{

class OpenGL4x : public Opengl2x {
  public:
    OpenGL4x();

    Device* createDevice( void * hwnd, const Options & opt ) const;
    void    deleteDevice( Device* d )  const;
  };

}

#endif // OPENGL4X_H
