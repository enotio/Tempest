#ifndef CAMERA_H
#define CAMERA_H

#include <Tempest/AbstractCamera>
#include <Tempest/Matrix4x4>

namespace Tempest{

class Camera : public Tempest::AbstractCamera{
  public:
    Camera();

    virtual Matrix4x4 view() const;
    virtual Matrix4x4 projective() const;

    void setSpinX( double x );
    void setSpinY( double y );

    void setZoom( double z );

    double spinX() const;
    double spinY() const;

    double zoom() const;

    void setPerspective( bool use, int w = 1, int h = 1 );
    void setPerspective( int w, int h,
                         float use = 45.0,
                         float zmin = 0.1f, float zmax = 100.0f );

    void setPosition( double x, double y, double z );
    double x() const;
    double y() const;
    double z() const;

    void setDistance( double d );
    double distance() const;
  private:
    double sx,sy, m_zoom, dist;
    double pos[3];

    Tempest::Matrix4x4 mProj, mView;

    void updateView();
  };

}

#endif // CAMERA_H
