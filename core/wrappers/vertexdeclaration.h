#ifndef VERTEXDECLARATION_H
#define VERTEXDECLARATION_H

#include <vector>
#include <utility>

namespace Tempest {

class Device;

namespace Decl{

enum ComponentType{
  float0 = 0, // just trick
  float1 = 1,
  float2 = 2,
  float3 = 3,
  float4 = 4,

  color  = 5,
  short2 = 6,
  short4 = 7,

  half2  = 8,
  half4  = 9,
  count
  };

}

namespace Usage{

enum UsageType{
  Position = 0,
  BlendWeight,   // 1
  BlendIndices,  // 2
  Normal,        // 3
  PSize,         // 4
  TexCoord,      // 5
  Tangent,       // 6
  BiNormal,      // 7
  TessFactor,    // 8
  PositionT,     // 9
  Color,         // 10
  Fog,           // 11
  Depth,         // 12
  Sample,        // 13
  Count
  };

}

class VertexDeclaration {
  public:
    class Declarator;

    VertexDeclaration();
    VertexDeclaration( Device & de );
    VertexDeclaration( Device & de, const Declarator & d );
    VertexDeclaration( const VertexDeclaration & d );
    virtual ~VertexDeclaration();

    VertexDeclaration& operator = ( const VertexDeclaration & v );

    class Declarator{
      public:
        struct Element{
          Tempest::Decl::ComponentType component;
          Tempest::Usage::UsageType    usage;
          int index;


          bool operator == ( const Element& d ) const;
          bool operator != ( const Element& d ) const;
          };

        Declarator& operator << ( const Element& e );
        Declarator& add ( const Element& e );
        Declarator& add ( Tempest::Decl::ComponentType,
                          Tempest::Usage::UsageType, int id = 0 );

        int size() const;

        const Element& operator [] (int i) const;
        bool operator == ( const Declarator& d ) const;
        bool operator != ( const Declarator& d ) const;
      private:
        std::vector< Element > data;
      };

    const Declarator& declarator() const;
    bool isValid() const;
  private:
    struct Data{
      void* impl;
      Declarator decl;
      int count;
      } * decl;
    Device * dev;

    void delRef();

    friend class Device;
  };

template< class V1, class V2 >
bool vertexCast( V1& v1,
                 const VertexDeclaration::Declarator& v1d,
                 const V2& v2,
                 const VertexDeclaration::Declarator& v2d ) {
  if( v1d.size()!=v2d.size() )
    return 0;

  static const int sz[] = {
    0,1,2,3,4,
    4,2,4,
    2,4
    };

  static const int byte[] = {
    0,1*4,2*4,3*4,4*4,
    4,2*2,2*4,
    2*2,2*4
    };
  (void)byte;

  for( int i=0; i<v1d.size(); ++i ){
    if( v1d[i].usage!=v2d[i].usage )
      return 0;

    if( sz[ v1d[i].component ] != sz[ v2d[i].component ]  )
      return 0;
    }

  //char      * b1 = &v1;
  //const char* b2 = &v2;

  for( int i=0; i<v1d.size(); ++i ){
    //TODO
    }

  return 1;
  }

}

#endif // VERTEXDECLARATION_H
