#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <Tempest/VertexBuffer>
#include <Tempest/VertexDeclaration>
#include <Tempest/VertexBufferHolder>

#include <stdint.h>
#include <fstream>

namespace Tempest{

class Device;
class IndexBufferHolder;


struct DefaultVertex{
  float x,y,z;
  float u,v;
  float normal[3];
  };

struct ModelBounds{
  float min[3], max[3], mid[3];
  float diameter() const;
  float radius() const;
  };

template< class V = DefaultVertex >
struct RawModel{
  RawModel():hasIndex(false){}

  typedef V Vertex;
  typedef std::vector<Vertex> VertexList;

  VertexList vertex;

  bool hasIndex;
  typedef  std::vector<uint32_t> IndexList;
  IndexList index;

  ModelBounds computeBoundRect() const{
    return computeBoundRect(vertex);
    }

  static ModelBounds computeBoundRect( const VertexList& vertex ){
    ModelBounds r;
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
      float v[3] = { vertex[i].x, vertex[i].y, vertex[i].z };
      for( int q=0; q<3; ++q ){
        r.min[q] = std::min( r.min[q], v[q] );
        r.max[q] = std::max( r.max[q], v[q] );
        }
      }

    for( int i=0; i<3; ++i )
      r.mid[i] = 0.5*(r.min[i]+r.max[i]);

    return r;
    }
  };

template< class ModelVertex = DefaultVertex >
class Model {
  public:
    Model(){
      m_size = 0;

      for( size_t i=0; i<3; ++i ){
        bds.min[i] = 0;
        bds.mid[i] = 0;
        bds.max[i] = 0;
        }
      }

    virtual ~Model(){}

    typedef ModelVertex Vertex;
    typedef RawModel<Vertex> Raw;

    static Raw  loadRawData( const std::string& fname  ){
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

    static bool saveRawData( const std::string& fname, const Raw &r  ){
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

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & /*iboHolder*/,
               const std::vector<Vertex>& buf,
               const Tempest::VertexDeclaration::Declarator& decl ){
      vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

      m_size = buf.size();
      vbo = vboHolder.load( buf.data(), buf.size() );

      bds = Raw::computeBoundRect( buf );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const Raw& data,
               const Tempest::VertexDeclaration::Declarator& decl ){
      load( vboHolder, iboHolder, data.vertex, decl );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::string&        fname ){
      Model::Raw r = loadRawData( fname );
      Tempest::VertexDeclaration::Declarator decl;
      decl.add( Tempest::Decl::float3, Tempest::Usage::Position )
          .add( Tempest::Decl::float2, Tempest::Usage::TexCoord )
          .add( Tempest::Decl::float3, Tempest::Usage::Normal   );

      load( vboHolder, iboHolder, r.vertex, decl );
      }

    const VertexBuffer<Vertex> &vertexes() const{
      return vbo;
      }

    const Tempest::VertexDeclaration&   declaration() const{
      return vdecl;
      }

    int size() const{
      return m_size/3;
      }

    int vertexCount() const{
      return m_size;
      }

    const ModelBounds& bounds() const{
      return bds;
      }

    Tempest::AbstractAPI::PrimitiveType primitiveType() const{
      return Tempest::AbstractAPI::Triangle;
      }

    Model<ModelVertex> slice( int first, int size ) const {
      Model<ModelVertex> m = *this;
      m.vbo = m.vbo.slice( first, size );
      m.m_size = m.vbo.size();

      // TODO bsd recalc
      return m;
      }

    Model<ModelVertex> slice( int first ) const {
      Model<ModelVertex> m = *this;
      m.vbo = m.vbo.slice( first );
      m.m_size = m.vbo.size();

      // TODO bsd recalc
      return m;
      }

  private:
    Tempest::VertexBuffer<Vertex>   vbo;
    Tempest::VertexDeclaration    vdecl;
    ModelBounds bds;

    int m_size;
  };

}

#endif // MODEL_H
