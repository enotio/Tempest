#include "uniformtable.h"

#include <Tempest/Device>
#include <Tempest/Render>

Tempest::UniformTable::UniformTable( Tempest::Device & d,
                                  Tempest::VertexShader &   v,
                                  Tempest::FragmentShader & f )
      :device(d), vs(v), fs(f){

  }

Tempest::UniformTable::UniformTable(Tempest::Render &r)
                   :device( r.device ), vs( r.vsh ), fs( r.fsh ){

  }

void Tempest::UniformTable::addImpl( const Tempest::Matrix4x4 &m,
                                  const char *name,
                                  Tempest::UniformTable::Taget t) {
  if( t==Vertex )
    device.setUniform( vs, m, name ); else
    device.setUniform( fs, m, name );
  }

void Tempest::UniformTable::addImpl( const Tempest::Texture2d &tex,
                                  const char *name,
                                  Tempest::UniformTable::Taget t ) {
  if( t==Vertex ){
    //device.setUniform( vs, tex, name );
    } else {
    device.setUniform( fs, tex, name );
    }
  }

void Tempest::UniformTable::addImpl( double v,
                                  const char *name,
                                  Tempest::UniformTable::Taget t ) {
  if( t==Vertex ){
    device.setUniform( vs, &v, 1, name );
    } else {
    device.setUniform( fs, &v, 1, name );
    }
  }

void Tempest::UniformTable::addImpl( const double v[],
                                  int sz,
                                  const char *name,
                                  Tempest::UniformTable::Taget t ) {
  if( t==Vertex ){
    device.setUniform( vs, v, sz, name );
    } else {
    device.setUniform( fs, v, sz, name );
    }
  }
