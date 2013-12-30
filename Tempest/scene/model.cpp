#include "model.h"

#include <cmath>

using namespace Tempest;

float ModelBounds::diameter() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-min[i])*(max[i]-min[i]);

  return sqrt(s);
  }

float ModelBounds::radius() const {
  double s = 0;
  for( int i=0; i<3; ++i )
    s += (max[i]-mid[i])*(max[i]-mid[i]);

  return sqrt(s);
  }

const VertexDeclaration::Declarator &DefaultVertex::decl() {
  static const Tempest::VertexDeclaration::Declarator d = mkDecl();
  return d;
  }

VertexDeclaration::Declarator DefaultVertex::mkDecl() {
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::float3, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord )
      .add( Tempest::Decl::float3, Tempest::Usage::Normal   );

  return decl;
  }
