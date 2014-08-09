#ifndef ABSTRACTGRAPHICOBJECT_H
#define ABSTRACTGRAPHICOBJECT_H

#include <vector>
#include <typeinfo>

#include <Tempest/Model>
#include <Tempest/AbstractScene>

namespace Tempest{

class AbstractMaterial;

template< class V >
class Model;
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

    virtual const ModelBounds& bounds() const = 0;
    virtual const Matrix4x4&   transform() const = 0;

    bool isVisible() const{
      return visible;
      }

    void setVisible( bool v){
      //if( visible==v )
        //return;

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
    virtual void onTransformChanged( const Tempest::Matrix4x4& old ) const = 0;
  private:
    bool visible;
  };

template< class Material,
          class UserState = char >
class AbstractGraphicObject : public AbstractSceneObject {
  public:
    typedef AbstractScene< AbstractGraphicObject<Material,UserState> > Scene;

    AbstractGraphicObject( Scene & s ):AbstractSceneObject(), scene(&s){
      setVisible(1);
      vboH = 0;
      iboH = 0;

      isInScene = false;
      }

    AbstractGraphicObject( const AbstractGraphicObject& obj ):
        AbstractSceneObject(obj), scene( obj.scene ){
      setVisible(1);      
      isInScene = false;

      *this = obj;
      }

    virtual ~AbstractGraphicObject(){
      }

    AbstractGraphicObject& operator = ( const AbstractGraphicObject& obj ){
      if( this==&obj )
        return *this;

      scene = obj.scene;
      mat   = obj.mat;
      vboH  = obj.vboH;
      iboH  = obj.iboH;

      AbstractSceneObject::operator =( obj );

      return *this;
      }

    void setMaterial( const Material& m ){
      sceneDelObject();
      mat = m;
      sceneAddObject();
      }

    const Material& material() const {
      return mat;
      }

    size_t vboHandle() const{
      return vboH;
      }

    size_t iboHandle() const{
      return iboH;
      }

    UserState ustate;
  protected:
    void onTransformChanged( const Tempest::Matrix4x4& old ) const {
      if( isInScene )
        scene->onObjectTransform( this, old );
      }

    void sceneAddObject(){
      if( !isInScene ){
        scene->addObject( this );
        isInScene = true;
        }
      }

    void sceneDelObject(){
      if( isInScene ){
        scene->delObject( this );
        isInScene = false;
        }
      }

    size_t vboH, iboH;
  private:
    Material mat;
    Scene * scene;
    bool isInScene;

    //friend class Scene;
    friend class Render;
  };

}

#endif // ABSTRACTGRAPHICOBJECT_H
