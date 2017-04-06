#ifndef OPENGL4X_H
#define OPENGL4X_H

#include <Tempest/Opengl2x>

namespace Tempest{

class Opengl4x : public Opengl2x {
  public:
    Opengl4x();

    Device* allocDevice( void * hwnd, const Options & opt ) const;
    void    freeDevice( Device* d )  const;

  protected:
    struct ImplDevice;

    bool createContext(Detail::ImplDeviceBase *, void *hwnd, const Options &opt) const;
  };

}

#endif // OPENGL4X_H
