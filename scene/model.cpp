#include "model.h"

#include <fstream>
#include <stdint.h>
#include <cmath>
#include <cassert>

#include <Tempest/VertexBufferHolder>

using namespace Tempest;
/*
Model::Model(){
  m_size = 0;
  }

int Model::size() const {
  return m_size/3;
  }


const Model::Bounds &Model::bounds() const {
  return bds;
  }
*/
/*
Tempest::AbstractAPI::PrimitiveType Model::primitiveType() const {
  return Tempest::AbstractAPI::Triangle;
  }

const VertexBuffer<Model::Vertex> &Model::vertexes() const {
  return vbo;
  }

const Tempest::VertexDeclaration& Model::declaration() const {
  return vdecl;
  }

void Model::load( Tempest::VertexBufferHolder &vboHolder,
                  Tempest::IndexBufferHolder  &iboHolder,
                  const std::string &fname ){
  Model::Raw r = loadRawData( fname );

  load( vboHolder, iboHolder, r.vertex );
  }

void Model::load ( Tempest::VertexBufferHolder & vboHolder,
                   Tempest::IndexBufferHolder  & iboHolder,
                   const std::vector<Vertex>& buf ){
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::double3, Tempest::Usage::Position )
      .add( Tempest::Decl::double2, Tempest::Usage::TexCoord )
      .add( Tempest::Decl::double3, Tempest::Usage::Normal   );

  vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

  m_size = buf.size();
  vbo = vboHolder.load( buf.data(), buf.size() );

  bds = Raw::computeBoundRect( buf );
  }*/
/*
void Model::load( VertexBufferHolder &vboHolder,
                  IndexBufferHolder  &iboHolder,
                  const Model::Raw   &data ) {
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::double3, Tempest::Usage::Position )
      .add( Tempest::Decl::double2, Tempest::Usage::TexCoord )
      .add( Tempest::Decl::double3, Tempest::Usage::Normal   );

  int vsize = 3+2+3;
  size_t csize[ Tempest::Usage::Count ] = {};

  for( int i=Tempest::Usage::Position; i<Tempest::Usage::Count; ++i ){
    if( i!=Tempest::Usage::Position &&
        i!=Tempest::Usage::TexCoord &&
        i!=Tempest::Usage::Normal   ){
      csize[i] = data.udata[i].size()/data.vertex.size();
      decl.add( Tempest::Decl::ComponentType(Tempest::Decl::double0+csize[i]),
                Tempest::Usage::UsageType(i) );
      vsize += csize[i];
      }
    }

  vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

  m_size = data.vertex.size();
  std::vector<double> pack;
  pack.resize( m_size*vsize );

  for( size_t i=0, n=0; i<pack.size(); i+=vsize, ++n ){
    double *v = &pack[i];
    Vertex vstd = data.vertex[n];

    v[0] = vstd.x;
    v[1] = vstd.y;
    v[2] = vstd.z;

    v[3] = vstd.u;
    v[4] = vstd.v;

    v[5] = vstd.normal[0];
    v[6] = vstd.normal[1];
    v[7] = vstd.normal[2];

    v += 8;
    for( int u=Tempest::Usage::Position; u<Tempest::Usage::Count; ++u ){
      for( size_t r=0; r<csize[u]; ++r ){
        v[r] = data.udata[u][ n*csize[u]+r ];
        }
      v += csize[u];
      }
    }

  //vbo = vboHolder.load( pack.data(), pack.size() );

  bds = Raw::computeBoundRect( data.vertex );
  }*/

/*
Model::Raw Model::loadRawData( const std::string& fname  ){
  std::fstream fin( fname.data(), std::fstream::in | std::fstream::binary );

  char magic[6] = {};
  fin.read( magic, 5 );

  if( std::string(magic)!="Model" ){
    fin.close();

    Raw r;
    r.hasIndex = false;
    return r;
    }

  uint16_t ver  = 0,
           size = 0;

  fin.read( (char*)&ver,  sizeof(ver)  );
  fin.read( (char*)&size, sizeof(size) );

  Raw meat;
  meat.hasIndex = false;
  meat.vertex.resize( size );

  fin.read( (char*)&meat.vertex[0], size*sizeof(Vertex) );

  fin.close();

  return meat;
  }

bool Model::saveRawData( const std::string& fname, const Raw &r  ){
  std::fstream fout( fname.data(), std::fstream::out | std::fstream::binary );

  if( !fout.is_open() )
    return false;

  char magic[6] = "Model";
  fout.write( magic, 5 );

  uint16_t ver  = 0,
           size = r.vertex.size();

  fout.write( (char*)&ver,  sizeof(ver)  );
  fout.write( (char*)&size, sizeof(size) );

  fout.write( (char*)&r.vertex[0], size*sizeof(Vertex) );

  fout.close();
  return true;
  }
*/

/*Model::Bounds Model::Raw::computeBoundRect() const {
  return computeBoundRect(vertex);
  }

Model::Bounds Model::Raw::computeBoundRect(const Model::Raw::VertexList &vertex) {
  Model::Bounds r;
  if( vertex.size()==0 ){
    std::fill(r.min, r.min+3, 0);
    std::fill(r.max, r.max+3, 0);
    std::fill(r.mid, r.mid+3, 0);
    return r;
    }

  r.min[0] = vertex[0].x;
  r.min[1] = vertex[0].y;
  r.min[2] = vertex[0].z;
  std::copy( r.min, r.min+3, r.max );

  for( size_t i=0; i<vertex.size(); ++i ){
    double v[3] = { vertex[i].x, vertex[i].y, vertex[i].z };
    for( int q=0; q<3; ++q ){
      r.min[q] = std::min( r.min[q], v[q] );
      r.max[q] = std::max( r.max[q], v[q] );
      }
    }

  for( int i=0; i<3; ++i )
    r.mid[i] = 0.5*(r.min[i]+r.max[i]);

  return r;
  }
*/

double ModelBounds::diameter() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-min[i])*(max[i]-min[i]);

  return sqrt(s);
  }

double ModelBounds::radius() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-mid[i])*(max[i]-mid[i]);

  return sqrt(s);
  }
