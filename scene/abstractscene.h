#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <vector>
#include <string>
#include <map>

#include <Tempest/Camera>
#include <Tempest/AbstractMaterial>
#include <Tempest/LightCollection>
#include <Tempest/ViewTester>

#include <cstddef>
#include <algorithm>

namespace Tempest{

template< class M, class UserState >
class AbstractGraphicObject;

class Device;
class AbstractCamera;
//class AbstractMaterial;

class AbstractRenderPass;
class ViewTester;

template< class Item >
class AbstractScene {
  public:
    AbstractScene(){
      m_camera = 0;
      m_lights = 0;
      m_viewtest = 0;

      setCamera( Camera() );

      Tempest::LightCollection c;
      Tempest::DirectionLight l;
      l.setDirection( 1, 1, -1 );
      c.direction().push_back( l );

      setLightCollection( c );
      setViewTester( ViewTester() );
      }

    virtual ~AbstractScene(){
      delete m_camera;
      delete m_lights;
      delete m_viewtest;
      }

    template< class V >
    void setViewTester( const V & c ){
      pSet( m_viewtest, c );
      //m_viewtest->setScene(*this);
      }
    const ViewTester& viewTester() const{ return *m_viewtest; }

    template< class C >
    void setCamera( const C & c ){
      pSet( m_camera, c );
      }
    const AbstractCamera& camera() const{ return *m_camera; }

    template< class L >
    void setLightCollection( const L & l ){
      pSet( m_lights, l );
      }

    const AbstractLightCollection& lights() const{ return *m_lights; }
    AbstractLightCollection& lights() { return *m_lights; }

  protected:
    virtual void onObjectAdded    ( const Item* ){}
    virtual void onObjectRemoved  ( const Item* ){}
    virtual void onObjectTransform( const Item*, const Tempest::Matrix4x4& ){}

  private:
    template< class T, class A >
    static void pSet( T *& tg, const A & a ){
      (void)((const T&)a);

      if( tg==0 || !typeEQ(tg, &a) ){
        delete tg;
        tg = new A(a);
        } else {
        A * val = reinterpret_cast<A*>(tg);
        *val = a;
        }
      }

    AbstractScene( const AbstractScene & );
    AbstractScene & operator = ( const AbstractScene& );

    void addObject( Item* x ){
      onObjectAdded( x );
      }

    void delObject( Item* x ){
      onObjectRemoved(x);
      }

    static bool typeEQ( const void* a, const void* b ){
      return *((void**)a) == *((void**)b);
      }

    AbstractCamera* m_camera;
    AbstractLightCollection * m_lights;
    ViewTester              * m_viewtest;

    template< class M >
    friend class Tempest::AbstractGraphicObject;
    //friend class Item;
  };

}

#endif // ABSTRACTSCENE_H
