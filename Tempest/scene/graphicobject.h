#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H

#include <Tempest/AbstractGraphicObject>
#include <Tempest/Matrix4x4>
#include <Tempest/Model>
#include <Tempest/AbstractScene>
namespace Tempest{

class AbstractMaterial;

template< class Material,
          class UserState = char >
class GraphicObject : public AbstractGraphicObject<Material, UserState> {
  public:
    typedef AbstractScene< AbstractGraphicObject<Material,UserState> > Scene;

    GraphicObject( Scene & s ) : AbstractGraphicObject<Material, UserState>(s) {
      m_model = new ModelPtr<DefaultVertex>( Model<>() );

      pos[0] = 0;
      pos[1] = 0;
      pos[2] = 0;

      size[0] = 1;
      size[1] = 1;
      size[2] = 1;

      rx = 0;
      ry = 0;
      rz = 0;

      radi = 0;

      needToUpdateMat = true;

      this->sceneAddObject();
      }

    GraphicObject( const GraphicObject& obj ) : AbstractGraphicObject<Material, UserState>(obj) {
      m_model = new ModelPtr<DefaultVertex>( Model<>());
      m_model->~IModelPtr();

      const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(obj.m_model);
      m2->cloneTo(m_model);

      this->vboH = obj.vboH;
      this->iboH = obj.iboH;

      mat = obj.mat;
      std::copy( obj.pos,  obj.pos+3,  pos  );
      std::copy( obj.size, obj.size+3, size );

      rx = obj.rx;
      ry = obj.ry;
      rz = obj.rz;

      radi = obj.radi;

      mat = computeMat();
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

      AbstractGraphicObject<Material, UserState>::operator = (g);

      IModelPtr *m  = reinterpret_cast<IModelPtr*>(m_model);
      m->~IModelPtr();

      const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(g.m_model);
      m2->cloneTo(m);

      mat = g.mat;
      std::copy( g.pos,  g.pos+3,  pos  );
      std::copy( g.size, g.size+3, size );

      rx = g.rx;
      ry = g.ry;
      rz = g.rz;

      radi = g.radi;

      mat = computeMat();
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
      mat = computeMat();

      this->vboH = md.vertexes().handle();
      this->iboH = md.indexes().handle();
      this->sceneAddObject();
      }

    const ModelBounds& bounds() const {
      const IModelPtr *m  = reinterpret_cast< const IModelPtr*>(m_model);
      return m->bounds();
      }

    const Matrix4x4& transform() const {
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

    virtual void setRotation( double x, double y, double z ){
      if( x==rx && y==ry && z==rz )
        return;

      rx = x;
      ry = y;
      rz = z;

      needToUpdateMat = true;
      updateMat();
      }

    virtual float x() const {
      return pos[0];
      }

    virtual float y() const {
      return pos[1];
      }

    virtual float z() const {
      return pos[2];
      }

    virtual float sizeX() const {
      return size[0];
      }

    virtual float sizeY() const {
      return size[1];
      }

    virtual float sizeZ() const {
      return size[2];
      }

    virtual float angleX() const {
      return rx;
      }

    virtual float angleY() const{
      return ry;
      }

    virtual float angleZ() const{
      return rz;
      }

    virtual float radius() const {
      return radi;
      }

    virtual void render( Device& dev, ShaderProgram &p ) const{
      m_model->draw(dev, p);
      }

  protected:
    virtual Tempest::Matrix4x4 computeMat() const {
      Tempest::Matrix4x4 mat;
      mat.translate( pos );

      mat.rotate( rx, 1, 0, 0 );
      mat.rotate( ry, 0, 1, 0 );
      mat.rotate( rz, 0, 0, 1 );

      mat.scale( size[0], size[1], size[2] );

      return mat;
      }

    void requestMatrixUpdate(){
      needToUpdateMat = true;
      }

  private:
    struct IModelPtr{
      virtual ~IModelPtr(){}
      virtual const ModelBounds& bounds() const = 0;
      virtual void cloneTo( void* ) const = 0;
      virtual void draw( Device &r, ShaderProgram &sh ) const = 0;

      virtual size_t vboHandle() const = 0;
      virtual size_t iboHandle() const = 0;
      };

    template< class V >
    struct ModelPtr : public IModelPtr{
      ModelPtr( const Model<V> & m ):model(m){
        }

      Model<V> model;
      const ModelBounds& bounds() const { return model.bounds(); }
      virtual void cloneTo( void* p ) const {
        new(p) ModelPtr(model);
        }

      void draw( Device &dev, ShaderProgram &sh ) const {
        if( model.indexes().size() )
          dev.drawIndexed( model.primitiveType(),
                           sh,
                           model.declaration(),
                           model.vertexes(), model.indexes(),
                           0, 0,
                           model.primitiveCount() );
          else
          dev.drawPrimitive( model.primitiveType(),
                             sh,
                             model.declaration(),
                             model.vertexes(),
                             0,
                             model.primitiveCount()  );
        }

      size_t vboHandle() const{
        return model.vertexes().handle();
        }

      size_t iboHandle() const{
        return model.indexes().handle();
        }
      };

    IModelPtr* m_model;

    float pos[3], size[3], rx, ry, rz;

    mutable double radi;
    mutable Matrix4x4 mat;
    mutable bool needToUpdateMat;

    void updateMat() const {
      if( !needToUpdateMat )
        return;

      Tempest::Matrix4x4 oldM = mat;

      mat = computeMat();
      radi = (*std::max_element(size, size+3))*bounds().radius();
      needToUpdateMat = false;

      this->onTransformChanged(oldM);
      }
  };

}

#endif // GRAPHICOBJECT_H
