#ifndef VIEWTESTER_H
#define VIEWTESTER_H

#include <Tempest/Model>

namespace Tempest{

class AbstractSceneObject;

class Matrix4x4;
class AbstractCamera;
class Frustum;

class ViewTester {
  public:  
    enum VisibleStatus{
      NotVisible = 0,
      PartialVisible,
      FullVisible
      };

    ViewTester();
    virtual ~ViewTester();

    virtual bool isVisible( const AbstractSceneObject & obj,
                            const AbstractCamera & c ) const;

    virtual bool isVisible( const AbstractSceneObject & obj,
                            const Matrix4x4 & mvp ) const;

    virtual bool isVisible( const ModelBounds & obj,
                            const Matrix4x4 & mvp ) const;

    virtual bool isVisible( float x,
                            float y,
                            float z,
                            float r,
                            const Frustum &frustum ) const;

    virtual VisibleStatus checkVisible( float x,
                                        float y,
                                        float z,
                                        float r,
                                        const Frustum &frustum ) const;

  private:
    static inline bool rangeTest( double l, double x, double r ){
      return l<=x && x<=r;
      }

  friend class Scene;
  };

}
#endif // VIEWTESTER_H
