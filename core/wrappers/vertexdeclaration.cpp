#include "vertexdeclaration.h"

#include <Tempest/Device>

using namespace Tempest;


VertexDeclaration::VertexDeclaration():dev(0){
  decl = 0;
  //de.addVertexDeclaration( *this );
  }

VertexDeclaration::VertexDeclaration(Device &de):dev(&de){
  decl = 0;
  dev->addVertexDeclaration( *this );
  }

VertexDeclaration::VertexDeclaration( Device &de, const Declarator &d )
                  : dev(&de) {
  decl = new Data();

  decl->count = 1;
  decl->impl  = dev->createVertexDecl( d );
  decl->decl  = d;

  dev->addVertexDeclaration( *this );
  }

VertexDeclaration::VertexDeclaration(const VertexDeclaration &d):dev( d.dev ) {
  decl = d.decl;

  if( decl )
    ++decl->count;

  dev = d.dev;
  if( dev )
    dev->addVertexDeclaration( *this );
  }

VertexDeclaration::~VertexDeclaration(){
  delRef();

  if( dev )
    dev->delVertexDeclaration( *this );
  }

VertexDeclaration &VertexDeclaration::operator = ( const VertexDeclaration& v) {
  if( this==&v )
    return *this;
  if( dev )
    dev->delVertexDeclaration( *this );

  delRef();
  decl = v.decl;

  if( decl )
    ++decl->count;

  dev = v.dev;

  if( dev )
    dev->addVertexDeclaration( *this );
  return *this;
  }

void VertexDeclaration::delRef() {
  if( decl==0 )
    return;

  --decl->count;

  if( decl->count==0 ){
    if( dev )
      dev->deleteVertexDecl( (Tempest::AbstractAPI::VertexDecl*)decl->impl );
    delete decl;
    }
  }


VertexDeclaration::Declarator&
  VertexDeclaration::Declarator::operator <<( const Element & t ){
  data.push_back( t );

  return *this;
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add(const Element &e) {
  data.push_back( e );

  return *this;
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add( Tempest::Decl::ComponentType c,
                                      Usage::UsageType u,
                                      int id ) {
  Element e;
  e.component = c;
  e.usage     = u;
  e.index     = id;
  return add( e );
  }

int VertexDeclaration::Declarator::size() const{
  return data.size();
  }

const VertexDeclaration::Declarator::Element&
    VertexDeclaration::Declarator::operator [] (int i) const {
  return data[i];
  }
