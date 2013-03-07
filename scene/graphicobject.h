#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H

#include <Tempest/AbstractGraphicObject>
#include <Tempest/Matrix4x4>
#include <Tempest/Model>
#include <Tempest/Render>
#include <Tempest/AbstractScene>

namespace Tempest{

class AbstractMaterial;

template< class Material >
class GraphicObject : public AbstractGraphicObject<Material> {
  public:
    typedef AbstractScene< AbstractGraphicObject<Material> > Scene;

    GraphicObject( Scene & s ) : AbstractGraphicObject<Material>(s) {
      m_model = new ModelPtr<DefaultVertex>( Model<>());

      pos[0] = 0;
      pos[1] = 0;
      pos[2] = 0;

      size[0] = 1;
      size[1] = 1;
      size[2] = 1;

      rx = 0;
      rz = 0;

      radi = 0;

      needToUpdateMat = true;
      this->sceneAddObject();
      }

    GraphicObject( const GraphicObject& obj ) : AbstractGraphicObject<Material>(obj) {
      m_model = new ModelPtr<DefaultVertex>( Model<>());
      m_model->~IModelPtr();

      const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(obj.m_model);
      m2->cloneTo(m_model);

      mat = obj.mat;
      std::copy( obj.pos,  obj.pos+3,  pos  );
      std::copy( obj.size, obj.size+3, size );

      rx = obj.rx;
      rz = obj.rz;

      radi = obj.radi;

      computeMat();
      needToUpdateMat = false;
      this->sceneAddObject();
      }

    ~GraphicObject(){
      this->sceneDelObject();
      delete m_model;
      }

    GraphicObject& operator = ( const GraphicObject & g){
      if( this==&g )
        return *this;

      this->sceneDelObject();
      AbstractGraphicObject<Material>::operator = (g);

      IModelPtr *m  = reinterpret_cast<IModelPtr*>(m_model);
      m->~IModelPtr();

      const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(g.m_model);
      m2->cloneTo(m);

      mat = g.mat;
      std::copy( g.pos,  g.pos+3,  pos  );
      std::copy( g.size, g.size+3, size );

      rx = g.rx;
      rz = g.rz;

      radi = g.radi;

      computeMat();
      needToUpdateMat = false;
      this->sceneAddObject();

      return *this;
      }

    template< class V >
    void setModel( const Model<V> & md ){
      this->sceneDelObject();

      IModelPtr *m  = reinterpret_cast<IModelPtr*>(m_model);
      m->~IModelPtr();
      new (m_model) ModelPtr<V>(md);
      computeMat();

      this->sceneAddObject();
      }

    //const Model<>& model() const;
    ModelBounds bounds() const {
      const IModelPtr *m  = reinterpret_cast< const IModelPtr*>(m_model);
      return m->bounds();
      }

    Matrix4x4 transform() const {
      updateMat();
      return mat;
      }

    virtual void setPosition( double x, double y, double z ){
      if( pos[0]==x &&
          pos[1]==y &&
          pos[2]==z )
        return;

      pos[0] = x;
      pos[1] = y;
      pos[2] = z;

      needToUpdateMat = true;
      updateMat();
      }

    virtual void setSize    ( double x, double y, double z ){
      if( size[0]==x &&
          size[1]==y &&
          size[2]==z )
        return;

      size[0] = x;
      size[1] = y;
      size[2] = z;

      needToUpdateMat = true;
      updateMat();
      }

    virtual void setSize    ( double s ){
      setSize( s, s, s );
      }

    virtual void setRotation( double x, double z ){
      if( x==rx && z==rz )
        return;

      rx = x;
      rz = z;

      needToUpdateMat = true;
      updateMat();
      }

    virtual double x() const {
      return pos[0];
      }

    virtual double y() const {
      return pos[1];
      }

    virtual double z() const {
      return pos[2];
      }

    virtual double sizeX() const {
      return size[0];
      }

    virtual double sizeY() const {
      return size[1];
      }

    virtual double sizeZ() const {
      return size[2];
      }

    virtual double angleX() const {
      return rx;
      }

    virtual double angleZ() const{
      return rz;
      }

    virtual double radius() const {
      return radi;
      }
  private:
    struct IModelPtr{
      virtual ~IModelPtr(){}
      virtual ModelBounds bounds() const = 0;
      virtual void cloneTo( void* ) const = 0;
      virtual void draw( const AbstractMaterial &mat,
                         Render &r,
                         const Tempest::Matrix4x4 & object,
                         const AbstractCamera & camera) const = 0;

      virtual void draw( Render &r ) const = 0;
      };

    template< class V >
    struct ModelPtr : public IModelPtr{
      ModelPtr( const Model<V> & m ):model(m){
        }

      Model<V> model;
      ModelBounds bounds() const { return model.bounds(); }
      virtual void cloneTo( void* p ) const {
        new(p) ModelPtr(model);
        }

      void draw( const AbstractMaterial &mat,
                 Render &r,
                 const Tempest::Matrix4x4 & object,
                 const AbstractCamera & camera) const {
        r.draw( mat, model, object, camera );
        }

      void draw( Render &r ) const {
        r.draw( model );
        }
      };

    IModelPtr* m_model;

    void render( const AbstractMaterial &mat,
                 Render &r,
                 const Tempest::Matrix4x4 & object,
                 const AbstractCamera & camera ) const {
      //r.draw( mat, m_model, object, camera );
      const IModelPtr *m  = reinterpret_cast<const IModelPtr*>(m_model);
      return m->draw( mat, r, object, camera );
      }

    void render( Render &r ) const {
      //r.draw( mat, m_model, object, camera );
      const IModelPtr *m  = reinterpret_cast<const IModelPtr*>(m_model);
      return m->draw( r );
      }

    double pos[3], size[3], rx, rz;

    mutable double radi;
    mutable Matrix4x4 mat;
    mutable bool needToUpdateMat;

    void updateMat() const {
      if( !needToUpdateMat )
        return;

      Tempest::Matrix4x4 oldM = mat;
      computeMat();
      this->onTransformChanged(oldM);
      }

    void computeMat() const {
      mat.identity();
      mat.translate( pos );

      mat.rotate( rx, 1, 0, 0 );
      mat.rotate( rz, 0, 0, 1 );

      mat.scale( size[0], size[1], size[2] );

      radi = (*std::max_element(size, size+3))*bounds().radius();
      needToUpdateMat = false;
      }
  };

}

#endif // GRAPHICOBJECT_H
