#ifndef UNIFORMTABLE_H
#define UNIFORMTABLE_H

#include <Tempest/Texture2d>
#include <Tempest/Matrix4x4>

#include <string>
#include <utility>

namespace Tempest{

class Device;
class VertexShader;
class FragmentShader;

class Render;

class UniformTable {
  public:
    UniformTable( Device & d,
                  Tempest::VertexShader &   vs,
                  Tempest::FragmentShader & fs );

    UniformTable( Tempest::Render & r );

    enum Taget{
      Vertex   = 0,
      Fragment = 1,
      TagetsCount
      };

    template< class T >
    void add( const T& t,
              const char* name,
              Taget taget ){
      addImpl( t, name, taget);
      }

    template< class T >
    void add( const T& t,
              const std::string & name,
              Taget taget ){
      addImpl( t, name.data(), taget);
      }

    template< class T >
    void add( const T t[],
              int arrSize,
              const std::string & name,
              Taget taget ){
      addImpl( t, arrSize, name.data(), taget);
      }

  private:

    void addImpl( const Tempest::Texture2d& t,
                  const char* name,
                  Taget taget );

    void addImpl( const Tempest::Matrix4x4& t,
                  const char* name,
                  Taget taget );

    void addImpl( double t,
                  const char* name,
                  Taget taget );

    void addImpl( const double t[],
                  int sz,
                  const char* name,
                  Taget taget );

    Device & device;
    Tempest::VertexShader &   vs;
    Tempest::FragmentShader & fs;
  };

}

#endif // UNIFORMTABLE_H
