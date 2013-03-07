#include "graphicobject.h"

#include <Tempest/AbstractMaterial>
#include <Tempest/Render>

using namespace Tempest;

/*
GraphicObject::GraphicObject( Scene & s ):AbstractGraphicObject(s){
  m_model = new ModelPtr<DefaultVertex>( Model<>());

  pos[0] = 0;
  pos[1] = 0;
  pos[2] = 0;

  size[0] = 1;
  size[1] = 1;
  size[2] = 1;

  rx = 0;
  rz = 0;

  needToUpdateMat = true;
  }

GraphicObject:: GraphicObject(const GraphicObject &obj)
              : AbstractGraphicObject( obj ){
  m_model = new ModelPtr<DefaultVertex>( Model<>());
  m_model->~IModelPtr();

  const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(obj.m_model);
  m2->cloneTo(m_model);

  mat = obj.mat;
  std::copy( obj.pos,  obj.pos+3,  pos  );
  std::copy( obj.size, obj.size+3, size );

  rx = obj.rx;
  rz = obj.rz;
  needToUpdateMat = true;
  }

GraphicObject::~GraphicObject() {
  //const IModelPtr *m  = reinterpret_cast< const IModelPtr*>(m_model);
  //m->~IModelPtr();

  delete m_model;
  }

GraphicObject &GraphicObject::operator =(const GraphicObject &g) {
  if( this==&g )
    return *this;

  AbstractGraphicObject::operator = (g);

  IModelPtr *m  = reinterpret_cast<IModelPtr*>(m_model);
  m->~IModelPtr();

  const IModelPtr *m2  = reinterpret_cast< const IModelPtr*>(g.m_model);
  m2->cloneTo(m);

  mat = g.mat;
  std::copy( g.pos, g.pos+3, pos );
  std::copy( g.size, g.size+3, size );

  rx = g.rx;
  rz = g.rz;
  needToUpdateMat = true;

  return *this;
  }

ModelBounds GraphicObject::bounds() const {
  const IModelPtr *m  = reinterpret_cast< const IModelPtr*>(m_model);
  return m->bounds();
  }

Matrix4x4 GraphicObject::transform() const {
  updateMat();
  return mat;
  }

void GraphicObject::setPosition( double x, double y, double z ){
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  needToUpdateMat = true;
  }

void GraphicObject::setSize    ( double s ){
  setSize( s, s, s );
  }

void GraphicObject::setSize    ( double x, double y, double z ){
  size[0] = x;
  size[1] = y;
  size[2] = z;

  needToUpdateMat = true;
  }

void GraphicObject::setRotation( double x, double z ){
  rx = x;
  rz = z;

  needToUpdateMat = true;
  }

double GraphicObject::x() const {
  return pos[0];
  }

double GraphicObject::y() const {
  return pos[1];
  }

double GraphicObject::z() const {
  return pos[2];
  }

double GraphicObject::sizeX() const {
  return size[0];
  }

double GraphicObject::sizeY() const {
  return size[1];
  }

double GraphicObject::sizeZ() const {
  return size[2];
  }

double GraphicObject::angleX() const {
  return rx;
  }

double GraphicObject::angleZ() const {
  return rz;
  }

void GraphicObject::render( const AbstractMaterial & mat,
                            Render &r, const Matrix4x4 &object,
                            const AbstractCamera &camera ) const {
  //r.draw( mat, m_model, object, camera );
  const IModelPtr *m  = reinterpret_cast<const IModelPtr*>(m_model);
  return m->draw( mat, r, object, camera );
  }

void GraphicObject::updateMat() const{
  if( !needToUpdateMat )
    return;
  needToUpdateMat = false;

  mat.identity();
  mat.translate( pos );

  mat.rotate( rx, 1, 0, 0 );
  mat.rotate( rz, 0, 0, 1 );

  mat.scale( size[0], size[1], size[2] );
  }
*/
