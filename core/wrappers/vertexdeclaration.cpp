#include "vertexdeclaration.h"

#include <Tempest/Device>

using namespace Tempest;


VertexDeclaration::VertexDeclaration():dev(0){
  decl = 0;
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


VertexDeclaration::Declarator::Declarator():maxTexId(0){
  }

VertexDeclaration::Declarator&
  VertexDeclaration::Declarator::operator <<( const Element & t ){
  add( t );

  return *this;
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add(const Element &e) {
  data.push_back( e );

  if( e.usage==Usage::TexCoord ){
    maxTexId = std::max<size_t>( maxTexId, e.index+1 );
    }

  return *this;
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add( Tempest::Decl::ComponentType c,
                                      Tempest::Usage::UsageType attrName, int id ) {
  Element e;
  e.component = c;
  e.usage     = attrName;
  e.index     = id;
  return add( e );
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add(Tempest::Decl::ComponentType c,
                                      const char *attrName) {
  static const char* uType[] = {
    "Position",
    "BlendWeight",   // 1
    "BlendIndices",  // 2
    "Normal",        // 3
    "PSize",         // 4
    "TexCoord",      // 5
    "Tangent",       // 6
    "BiNormal",      // 7
    "TessFactor",    // 8
    "PositionT",     // 9
    "Color",         // 10
    "Fog",           // 11
    "Depth",         // 12
    "Sample",        // 13
    ""
    };

  for( int i=0; uType[i]; ++i )
    if( strcmp(uType[i],attrName)==0 ){
      return add( c, Usage::UsageType(i) );
      }

  Element e;
  e.component = c;
  e.attrName  = attrName;
  e.index     = 0;
  return add( e );
  }

int VertexDeclaration::Declarator::size() const{
  return data.size();
  }

const VertexDeclaration::Declarator::Element&
    VertexDeclaration::Declarator::operator [] (int i) const {
  return data[i];
  }

bool VertexDeclaration::Declarator::operator ==(const VertexDeclaration::Declarator &d) const {
  return data==d.data;
  }

bool VertexDeclaration::Declarator::operator !=(const VertexDeclaration::Declarator &d) const {
  return data!=d.data;
  }

size_t VertexDeclaration::Declarator::texCoordCount() const {
  return maxTexId;
  }

VertexDeclaration::Declarator::Element::Element()
  :component(Decl::count), usage(Usage::Count), index(0) {
  }

bool VertexDeclaration::Declarator::Element::operator ==(const VertexDeclaration::Declarator::Element &d) const {
  return component==d.component &&
         usage    ==d.usage &&
         index    ==d.index;
  }

bool VertexDeclaration::Declarator::Element::operator !=(const VertexDeclaration::Declarator::Element &d) const {
  return !(*this==d);
  }

const Tempest::VertexDeclaration::Declarator&
  Tempest::VertexDeclaration::declarator() const {
  return decl->decl;
  }

bool VertexDeclaration::isValid() const {
  return decl;
  }
