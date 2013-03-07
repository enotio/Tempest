#ifndef VIEWTESTER_H
#define VIEWTESTER_H

#include <Tempest/Model>

namespace Tempest{

class AbstractSceneObject;

class Matrix4x4;
class Scene;
class AbstractCamera;

class ViewTester {
  public:
    ViewTester();
    virtual ~ViewTester();

    virtual bool isVisible( const AbstractSceneObject & obj,
                            const AbstractCamera & c ) const;

    virtual bool isVisible( const AbstractSceneObject & obj,
                            const Matrix4x4 & mvp ) const;

    virtual bool isVisible( const ModelBounds & obj,
                            const Matrix4x4 & mvp ) const;
/*
    virtual bool isVisible( const Model::Bounds & obj,
                            const Matrix4x4 & vp,
                            double x, double y, double z ) const;
*/
  protected:
    // virtual void setScene( const Scene & ) {}

  private:
    static inline bool rangeTest( double l, double x, double r ){
      return l<=x && x<=r;
      }

  friend class Scene;
  };

}
#endif // VIEWTESTER_H
