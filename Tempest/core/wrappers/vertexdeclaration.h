#ifndef VERTEXDECLARATION_H
#define VERTEXDECLARATION_H

#include <vector>
#include <utility>
#include <string>
#include <memory>

#include <Tempest/Half>

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

    //! Vertex declaration initalizer
    class Declarator{
      public:
        Declarator();
        ~Declarator();

        struct Element{
          Element();
          Tempest::Decl::ComponentType component;
          Tempest::Usage::UsageType    usage;
          std::string attrName;
          int index;

          bool operator == ( const Element& d ) const;
          bool operator != ( const Element& d ) const;
          };

        Declarator& operator << ( const Element& e );
        Declarator& add ( const Element& e );
        Declarator& add ( Tempest::Decl::ComponentType,
                          Tempest::Usage::UsageType, int id = 0 );

        //GLSL only
        Declarator& add ( Tempest::Decl::ComponentType,
                          const char* attrName );

        int size() const;

        const Element& operator [] (int i) const;
        bool operator == ( const Declarator& d ) const;
        bool operator != ( const Declarator& d ) const;

        size_t texCoordCount() const;
      private:
        std::vector< Element > data;
        size_t maxTexId;
      };

    const Declarator& declarator() const;
    bool isValid() const;
  private:
    struct Data{
      void* impl;
      Declarator decl;
      };
    std::shared_ptr<Data> decl;
    Device * dev;

    friend class Device;
  };

namespace Detail{
  template< class T>
  void assign( T* ax, const T* bx, Decl::ComponentType t1, Decl::ComponentType t2 ){
    char &a = *ax;
    char &b = *bx;

    switch( t1 ){
      case Decl::float0:
      case Decl::float1:
      case Decl::float2:
      case Decl::float3:
      case Decl::float4:
        if( Decl::float0 <= t2 && t2<= Decl::float4 )
          (float&)a = (float&)b;

        if( Decl::short2 <= t2 && t2<= Decl::short4 )
          (float&)a = (short&)b;

        if( Decl::color == t2 )
          (float&)a = (unsigned char&)b;

        if( Decl::half2 <= t2 && t2<= Decl::half4 )
          (float&)a = (Half&)b;
        break;

      case Decl::color:
        if( Decl::float0 <= t2 && t2<= Decl::float4 )
          (unsigned char&)a = (float&)b;

        if( Decl::short2 <= t2 && t2<= Decl::short4 )
          (unsigned char&)a = (short&)b;

        if( Decl::color == t2 )
          (unsigned char&)a = (unsigned char&)b;

        if( Decl::half2 <= t2 && t2<= Decl::half4 )
          (unsigned char&)a = (Half&)b;
        break;

      case Decl::short2:
      case Decl::short4:
        if( Decl::float0 <= t2 && t2<= Decl::float4 )
          (short&)a = (float&)b;

        if( Decl::short2 <= t2 && t2<= Decl::short4 )
          (short&)a = (short&)b;

        if( Decl::color == t2 )
          (short&)a = (unsigned char&)b;

        if( Decl::half2 <= t2 && t2<= Decl::half4 )
          (short&)a = (Half&)b;
        break;

      case Decl::half2:
      case Decl::half4:
        if( Decl::float0 <= t2 && t2<= Decl::float4 )
          (Half&)a = (float&)b;

        if( Decl::short2 <= t2 && t2<= Decl::short4 )
          (Half&)a = (short&)b;

        if( Decl::color == t2 )
          (Half&)a = (unsigned char&)b;

        if( Decl::half2 <= t2 && t2<= Decl::half4 )
          (Half&)a = (Half&)b;
        break;

      case Decl::count:
        break;
      }
    }
  }

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

  char      * b1 = &v1;
  const char* b2 = &v2;

  for( int i=0; i<v1d.size(); ++i ){
    const VertexDeclaration::Declarator::Element & e1 = v1d[i];
    const VertexDeclaration::Declarator::Element & e2 = v2d[i];

    size_t rsz = sz[ e1.component ];
    for( int r=0; r<rsz; ++r ){
      Detail::assign(b1, b2, e1.component, e2.component);
      b1 += sz[e1.component];
      b2 += sz[e2.component];
      }
    }

  return 1;
  }

}

#endif // VERTEXDECLARATION_H
