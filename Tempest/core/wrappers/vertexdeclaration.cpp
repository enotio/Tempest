#include "vertexdeclaration.h"

#include <Tempest/Device>
#include <algorithm>

using namespace Tempest;

VertexDeclaration::VertexDeclaration():dev(0){
  }

VertexDeclaration::VertexDeclaration(Device &de):dev(&de){
  dev->addVertexDeclaration( *this );
  }

VertexDeclaration::VertexDeclaration( Device &de, const Declarator &d )
                  : decl( new Data() ), dev(&de) {
  decl->impl  = dev->createVertexDecl( d );
  decl->decl  = d;

  dev->addVertexDeclaration( *this );
  }

VertexDeclaration::VertexDeclaration(const VertexDeclaration &d)
  : decl(d.decl), dev( d.dev ) {
  if( dev )
    dev->addVertexDeclaration( *this );
  }

VertexDeclaration::~VertexDeclaration(){
  if( dev ){
    if( decl.unique() )
      dev->deleteVertexDecl( (Tempest::AbstractAPI::VertexDecl*)decl->impl );
    dev->delVertexDeclaration( *this );
    }
  }

VertexDeclaration &VertexDeclaration::operator = ( const VertexDeclaration& v) {
  if( this==&v )
    return *this;

  if( dev ){
    dev->delVertexDeclaration( *this );

    if( decl.unique() )
      dev->deleteVertexDecl( (Tempest::AbstractAPI::VertexDecl*)decl->impl );
    }

  decl = v.decl;
  dev  = v.dev;

  if( dev )
    dev->addVertexDeclaration( *this );
  return *this;
  }

/*
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
*/

VertexDeclaration::Declarator::Declarator():maxTexId(0){
}

VertexDeclaration::Declarator::~Declarator(){
}

VertexDeclaration::Declarator&
  VertexDeclaration::Declarator::operator <<( const Element & t ){
  add( t );

  return *this;
  }

VertexDeclaration::Declarator &
  VertexDeclaration::Declarator::add(const Element &e) {
  for( size_t i=0; i<data.size(); ++i ){
    const Element& ex = data[i];

    if( ex.usage==e.usage &&
        ex.index==e.index ){
      T_ASSERT_X(0, "Duplicate vertex declaration");
      }

    if( ex.usage==Usage::Count ){
      T_ASSERT_X(ex.attrName==e.attrName, "Duplicate vertex declaration");

      if( e.usage==Usage::TexCoord ){
        std::string tc = "TexCoord";

        int id = e.index;
        while( id>0 ){
          tc.push_back('0'+e.index%10);
          id /= 10;
          }
        T_ASSERT_X(tc==ex.attrName, "Duplicate vertex declaration");
        }
      }

    if( e.usage==Usage::Count && ex.usage==Usage::TexCoord ){
      std::string tc = "TexCoord";

      int id = ex.index;
      while( id>0 ){
        tc.push_back('0');
        id /= 10;
        }

      id = ex.index;
      size_t ix = tc.size()-1;
      while( id>0 ){
        tc[ix] = ('0'+id%10);
        --ix;
        id /= 10;
        }

      T_ASSERT_X(tc==e.attrName, "Duplicate vertex declaration");
      }
    }

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
  return decl.get()!=0;
  }
