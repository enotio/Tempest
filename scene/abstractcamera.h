#ifndef ABSTRACTCAMERA_H
#define ABSTRACTCAMERA_H

namespace Tempest{

class Matrix4x4;

class AbstractCamera {
  public:
    virtual ~AbstractCamera(){}

    virtual Matrix4x4 view() const = 0;
    virtual Matrix4x4 projective() const = 0;
  };

}

#endif // ABSTRACTCAMERA_H
