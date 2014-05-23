#ifndef OPENGLBASE_H
#define OPENGLBASE_H

#include <Tempest/AbstractAPI>
#include <string>

namespace Tempest{

class OpenGLBase : public AbstractAPI {
  public:
    OpenGLBase();

    std::string vendor( AbstractAPI::Device* d ) const;
    std::string renderer( AbstractAPI::Device* d ) const;

  protected:
    virtual bool setDevice( AbstractAPI::Device* ) const = 0;
    virtual bool errCk() const;

    virtual void setClearDepth(float z ) const;
  };
}

#endif // OPENGLBASE_H
