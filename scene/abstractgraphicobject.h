#ifndef ABSTRACTGRAPHICOBJECT_H
#define ABSTRACTGRAPHICOBJECT_H

#include <vector>
#include <typeinfo>

#include <Tempest/Model>
#include <Tempest/AbstractMaterial>
#include <Tempest/AbstractScene>

namespace Tempest{

class AbstractMaterial;

template< class V >
class Model;

class Scene;
class Render;

class AbstractSceneObject {
  public:
    AbstractSceneObject():visible(1){}
    AbstractSceneObject( const AbstractSceneObject& obj ):visible(1){ *this = obj; }
    virtual ~AbstractSceneObject(){}

    AbstractSceneObject& operator = ( const AbstractSceneObject& obj ){
      if( this==&obj )
        return *this;

      visible = obj.visible;

      return *this;
      }

    virtual ModelBounds bounds() const = 0;
    virtual const Matrix4x4&  transform() const = 0;

    bool isVisible() const{
      return visible;
      }

    void setVisible( bool v){
      if( visible==v )
        return;

      visible = v;
      }

    virtual float x() const = 0;
    virtual float y() const = 0;
    virtual float z() const = 0;

    virtual float sizeX() const = 0;
    virtual float sizeY() const = 0;
    virtual float sizeZ() const = 0;

    virtual float radius() const = 0;
  protected:
    virtual void render( const Tempest::AbstractMaterial & mat,
                         Tempest::Render & r,
                         const Tempest::Matrix4x4 & object,
                         const Tempest::AbstractCamera & camera ) const = 0;

    virtual void render( Tempest::Render & r ) const = 0;
    virtual void onTransformChanged( const Tempest::Matrix4x4& old ) const = 0;
  private:
    bool visible;

    friend class Scene;
    friend class Render;
  };

template< class Material >
class AbstractGraphicObject : public AbstractSceneObject {
  public:
    typedef AbstractScene< AbstractGraphicObject<Material> > Scene;

    AbstractGraphicObject( Scene & s ):AbstractSceneObject(), scene(&s){
      setVisible(1);
      }

    AbstractGraphicObject( const AbstractGraphicObject& obj ):
        AbstractSceneObject(obj), scene( obj.scene ){
      setVisible(1);
      *this = obj;
      }

    virtual ~AbstractGraphicObject(){
      }

    AbstractGraphicObject& operator = ( const AbstractGraphicObject& obj ){
      if( this==&obj )
        return *this;

      scene = obj.scene;
      mat   = obj.mat;

      AbstractSceneObject::operator =( obj );
      return *this;
      }

    void setMaterial( const Material& m ){
      scene->delObject(this);
      mat = m;
      scene->addObject(this);
      }

    const Material& material() const {
      return mat;
      }

  protected:
    void onTransformChanged( const Tempest::Matrix4x4& old ) const {
      scene->onObjectTransform( this, old );
      }

    void sceneAddObject(){
      scene->addObject( this );
      }

    void sceneDelObject(){
      scene->delObject( this );
      }

  private:
    Material mat;
    Scene * scene;

    //friend class Scene;
    friend class Render;
  };

}

#endif // ABSTRACTGRAPHICOBJECT_H
