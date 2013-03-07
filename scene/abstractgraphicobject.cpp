#include "abstractgraphicobject.h"

#include <Tempest/AbstractMaterial>

using namespace Tempest;
/*
AbstractGraphicObject::AbstractGraphicObject( Scene & s ):scene(&s){
  setVisible(1);
  }

AbstractGraphicObject::AbstractGraphicObject( const AbstractGraphicObject& obj ){
  setVisible(1);
  *this = obj;
  }

AbstractGraphicObject::~AbstractGraphicObject(){
  free();
  }

void AbstractGraphicObject::setMaterial( IMaterial* m ){
  mat.push_back( m );

  if( visible)
    scene->addObject( this, m->data, m->id );//typeid(*m->data).name() );
  }

const AbstractMaterial* AbstractGraphicObject::find( const AbstractMaterial::MaterialID & t ) const {
  for( size_t i=0; i<mat.size(); ++i )
    if( mat[i]->id==t ){
      return mat[i]->data;
      }

  return 0;
  }

AbstractMaterial* AbstractGraphicObject::find( const AbstractMaterial::MaterialID & t ){
  for( size_t i=0; i<mat.size(); ++i )
    if( mat[i]->id==t ){
      return mat[i]->data;
      }

  return 0;
  }

void AbstractGraphicObject::free(){
  if( visible ){
    for( size_t i=0; i<mat.size(); ++i ){
      scene->delObject( this, mat[i]->id );
      }
    }

  for( size_t i=0; i<mat.size(); ++i ){
    delete mat[i];
    }

  mat.clear();
  }

bool AbstractGraphicObject::remove( AbstractMaterial::MaterialID t ){
  for( size_t i=0; i<mat.size(); ++i )
    if( mat[i]->id==t ){
      if( visible )
        scene->delObject( this, t );

      delete mat[i];
      for( size_t r=i; r+1 < mat.size(); ++r )
        mat[r] = mat[r+1];
      mat.pop_back();
      return 1;
      }

  return 0;
  }

AbstractGraphicObject& AbstractGraphicObject::
      operator = ( const AbstractGraphicObject& obj ){
  if( this==&obj )
    return *this;

  free();

  scene   = obj.scene;
  visible = obj.visible;

  for( size_t i=0; i<obj.mat.size(); ++i ){
    setMaterial( obj.mat[i]->clone() );
    }

  return *this;
  }

bool AbstractGraphicObject::isVisible() const {
  return visible;
  }

void AbstractGraphicObject::setVisible( bool v ) {
  if( visible==v )
    return;

  visible = v;
  if( !visible ){
    for( size_t i=0; i<mat.size(); ++i ){
      scene->delObject( this, mat[i]->id );
      }
    } else {
    for( size_t i=0; i<mat.size(); ++i ){
      scene->addObject( this, mat[i]->data, mat[i]->id );
      }
    }
  }

size_t AbstractGraphicObject::id() const {
  return myID;
  }
*/
