#include "shaderinput.h"

#include <cassert>

using namespace Tempest;

void ShaderInput::set(const char *name, const Texture2d &t) {
  tex.set( name, &t );
  }

void ShaderInput::set(const char *name, const Matrix4x4 &t) {
  mat.set( name, t );
  }

void ShaderInput::set(const char *name, float x) {
  Vec<1> v;
  v.v[0] = x;

  v1.set(name, v);
  }

void ShaderInput::set(const char *name, float x, float y) {
  Vec<2> v;
  v.v[0] = x;
  v.v[1] = y;

  v2.set(name, v);
  }

void ShaderInput::set(const char *name, float x, float y, float z) {
  Vec<3> v;
  v.v[0] = x;
  v.v[1] = y;
  v.v[2] = z;

  v3.set(name, v);
  }

void ShaderInput::set(const char *name, float x, float y, float z, float w) {
  Vec<4> v;
  v.v[0] = x;
  v.v[1] = y;
  v.v[2] = z;
  v.v[3] = w;

  v4.set(name, v);
  }

void ShaderInput::set(const char *name, const float *v, int l) {
  switch( l ){
    case 1: set( name, v[0] ); break;
    case 2: set( name, v[0], v[1] ); break;
    case 3: set( name, v[0], v[1], v[2] ); break;
    case 4: set( name, v[0], v[1], v[2], v[3] ); break;
    default: assert(0 & l);
    }
  }

void ShaderInput::set( const Uniform<float[2]> &u ) {
  set(u.name().data(), u[0], u[1]);
  }

void ShaderInput::set( const Uniform<float[3]> &u ) {
  set(u.name().data(), u[0], u[1], u[2]);
  }

void ShaderInput::set( const Uniform<Texture2d> &u ) {
  set(u.name().data(), *u.value());
  }

