#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>

#include <Tempest/VertexDeclaration>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/File>

#include <stdint.h>

namespace Tempest{

class Device;
class IndexBufferHolder;


struct DefaultVertex{
  float x,y,z;
  float u,v;
  float normal[3];

  static const Tempest::VertexDeclaration::Declarator& decl();

  private:
    static Tempest::VertexDeclaration::Declarator mkDecl();
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
  typedef  std::vector<uint16_t> IndexList;
  IndexList index;

  ModelBounds computeBoundRect() const{
    return computeBoundRect(vertex);
    }

  static ModelBounds computeBoundRect( const VertexBuffer<Vertex>& v ){
    return computeBoundRect( v.const_data(), v.size() );
    }

  static ModelBounds computeBoundRect( const VertexList& vertex ){
    return computeBoundRect(&vertex[0], vertex.size());
    }

  static ModelBounds computeBoundRect( const Vertex* vertex, size_t vsize ){
    ModelBounds r;
    if( vertex==0 || vsize==0 ){
      std::fill(r.min, r.min+3, 0);
      std::fill(r.max, r.max+3, 0);
      std::fill(r.mid, r.mid+3, 0);
      return r;
      }

    r.min[0] = vertex[0].x;
    r.min[1] = vertex[0].y;
    r.min[2] = vertex[0].z;
    std::copy( r.min, r.min+3, r.max );

    for( size_t i=1; i<vsize; ++i ){
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
      m_size   = 0;
      m_pcount = 0;

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
      RFile fin( fname );

      char magic[6] = {};
      fin.readData( magic, 5 );

      if( strcmp(magic,"Model")!=0 ){
        Raw r;
        r.hasIndex = false;
        return r;
        }

      uint16_t ver  = 0,
               size = 0;

      fin.readData( (char*)&ver,  sizeof(ver)  );
      fin.readData( (char*)&size, sizeof(size) );

      Raw meat;
      meat.hasIndex = false;
      meat.vertex.resize( size );

      fin.readData( (char*)&meat.vertex[0], size*sizeof(Vertex) );
      return meat;
      }

    static bool saveRawData( const std::string& fname, const Raw &r  ){
      WFile fout( fname.data() );

      if( !fout.isOpen() )
        return false;

      char magic[6] = "Model";
      fout.writeData( magic, 5 );

      uint16_t ver  = 0,
               size = r.vertex.size();

      fout.writeData( (char*)&ver,  sizeof(ver)  );
      fout.writeData( (char*)&size, sizeof(size) );

      fout.writeData( (char*)&r.vertex[0], size*sizeof(Vertex) );
      return true;
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & /*iboHolder*/,
               const Vertex* buf, size_t bufSize,
               const Tempest::VertexDeclaration::Declarator& decl,
               AbstractAPI::BufferFlag  bufferFlag = AbstractAPI::BF_NoFlags ){
      vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

      setupMSZ( bufSize );
      vbo = vboHolder.load( buf, bufSize, bufferFlag );

      bds = Raw::computeBoundRect( buf, bufSize );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::vector<Vertex>& buf,
               const Tempest::VertexDeclaration::Declarator& decl,
               AbstractAPI::BufferFlag  bufferFlag = AbstractAPI::BF_NoFlags ){
      load( vboHolder, iboHolder, &buf[0], buf.size(), decl, bufferFlag );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const Vertex*   buf, size_t bufSize,
               const uint16_t* index,size_t iboSize,
               const Tempest::VertexDeclaration::Declarator& decl,
               AbstractAPI::BufferFlag  bufferFlag = AbstractAPI::BF_NoFlags ){
      vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

      if( bufSize==0 )
        setupMSZ(0); else
        setupMSZ( iboSize );
      vbo = vboHolder.load( buf,   bufSize, bufferFlag );
      ibo = iboHolder.load( index, iboSize, bufferFlag );

      bds = Raw::computeBoundRect( buf, bufSize );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::vector<Vertex>&   buf,
               const std::vector<uint16_t>& index,
               const Tempest::VertexDeclaration::Declarator& decl,
               AbstractAPI::BufferFlag  bufferFlag = AbstractAPI::BF_NoFlags ){
      vdecl = Tempest::VertexDeclaration( vboHolder.device(), decl );

      if( buf.size()==0 )
        setupMSZ(0); else
        setupMSZ( index.size() );

      vbo = vboHolder.load( buf,   bufferFlag );
      ibo = iboHolder.load( index, bufferFlag );

      bds = Raw::computeBoundRect( buf );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const Raw& data,
               const Tempest::VertexDeclaration::Declarator& decl,
               AbstractAPI::BufferFlag  bufferFlag = AbstractAPI::BF_NoFlags ){
      if( data.hasIndex )
        load( vboHolder, iboHolder, data.vertex, data.index, decl, bufferFlag ); else
        load( vboHolder, iboHolder, data.vertex, decl, bufferFlag );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::string&        fname ){
      Model::Raw r = loadRawData( fname );
      load( vboHolder, iboHolder, r.vertex, DefaultVertex::decl() );
      }

    void load( const Tempest::VertexBuffer<ModelVertex> & v,
               const Tempest::IndexBuffer<uint16_t>     & i,
               const Tempest::VertexDeclaration   & d ){
      if( v.size()==0 )
        setupMSZ(0); else
        setupMSZ(i.size());

      vbo   = v;
      ibo   = i;
      vdecl = d;

      bds = Raw::computeBoundRect( v );
      }

    void load( const Tempest::VertexBuffer<ModelVertex> & v,
               const Tempest::VertexDeclaration   & d ){
      setupMSZ( v.size() );
      vbo   = v;
      ibo   = IndexBuffer<uint16_t>();
      vdecl = d;

      bds = Raw::computeBoundRect( v );
      }

    const VertexBuffer<Vertex> &vertexes() const{
      return vbo;
      }

    const IndexBuffer<uint16_t> &indexes() const{
      return ibo;
      }

    const Tempest::VertexDeclaration& declaration() const{
      return vdecl;
      }

    int size() const{
      return m_pcount;
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

    size_t primitiveCount() const {
      return m_pcount;
      }

    Model<ModelVertex> slice( int first, int size ) const {
      Model<ModelVertex> m = *this;

      if( m.ibo.size() )
        m.ibo = m.ibo.slice( first, size ); else
        m.vbo = m.vbo.slice( first, size );

      m.setupMSZ(size);

      m.bds = Raw::computeBoundRect( m.vbo );
      return m;
      }

    Model<ModelVertex> slice( int first ) const {
      int sz = m_size-first;
      return slice(first, sz);
      }

  private:
    Tempest::VertexBuffer<Vertex>   vbo;
    Tempest::IndexBuffer<uint16_t>  ibo;

    Tempest::VertexDeclaration    vdecl;
    ModelBounds bds;

    void setupMSZ( int sz ){
      m_size   = sz;
      m_pcount = AbstractAPI::primitiveCount( vertexCount(), primitiveType() );
      }
    int m_size, m_pcount;
  };

}

#endif // MODEL_H
