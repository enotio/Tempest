#ifndef ABSTRACTCAMERA_H
#define ABSTRACTCAMERA_H

namespace Tempest{

class Matrix4x4;
class Frustum;

class AbstractCamera {
  public:
    virtual ~AbstractCamera(){}

    virtual Matrix4x4 view() const = 0;
    virtual Matrix4x4 projective() const = 0;

    virtual Matrix4x4 viewProjective() const;
    virtual Frustum   frustum() const;
  };

class Frustum {
  public:
    Frustum();
    Frustum( const Tempest::AbstractCamera& c );
    Frustum( const Tempest::Matrix4x4& c );

    void fromMatrix( const Tempest::Matrix4x4& c );

    const float* plane(int i) const;
  private:
    float f[6][4];
  };

}

#endif // ABSTRACTCAMERA_H
