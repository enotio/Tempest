#ifndef VERTEXDECLARATION_H
#define VERTEXDECLARATION_H

#include <vector>
#include <utility>

namespace Tempest {

class Device;

namespace Decl{

enum ComponentType{
  double0 = 0, // just trick
  double1 = 1,
  double2 = 2,
  double3 = 3,
  double4 = 4,

  color  = 5,
  short2 = 6,
  short4 = 7
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
          };

        Declarator& operator << ( const Element& e );
        Declarator& add ( const Element& e );
        Declarator& add ( Tempest::Decl::ComponentType,
                          Tempest::Usage::UsageType, int id = 0 );

        int size() const;

        const Element& operator [] (int i) const;
      private:
        std::vector< Element > data;
      };

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

}

#endif // VERTEXDECLARATION_H
