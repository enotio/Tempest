#ifndef UNIFORM_H
#define UNIFORM_H

#include <Tempest/Light>
#include <Tempest/Texture2d>
#include <string>

namespace Tempest{

template< class T >
class Uniform {
  private:
    Uniform(){}
    Uniform( const Uniform& ){}
  };

enum LightParam{
  Direction,
  Position,
  LightColor,
  LightAblimient
  };

namespace Detail{
  struct ShInput{
    // TODO: uniform int-name cash
    };
  }

template<>
class Uniform<float[2]> {
  public:
    void setName( const std::string& n ){
      mname = n;
      }

    const std::string& name() const {
      return mname;
      }

    void set( double x, double y ){
      data[0] = x;
      data[1] = y;
      }

    void set( const double x[2] ){
      data[0] = x[0];
      data[1] = x[1];
      }

    float operator [] ( int i ) const {
      return data[i];
      }

  private:
    std::string mname;
    float data[2];

    mutable Detail::ShInput sinput;

  friend class Device;
  };

template<>
class Uniform<float[3]> {
  public:
    void setName( const std::string& n ){
      mname = n;
      }

    const std::string& name() const {
      return mname;
      }

    void set( double x, double y, double z ){
      data[0] = x;
      data[1] = y;
      data[2] = z;
      }

    void set( const double x[3] ){
      data[0] = x[0];
      data[1] = x[1];
      data[2] = x[2];
      }

    void set( const Tempest::DirectionLight & d, LightParam p ){
      if( p==Direction )
        set( d.xDirection(), d.yDirection(), d.zDirection() );

      if( p==Position )
        set( 0, 0, 0 );

      if( p==LightColor )
        set( d.color().r(), d.color().g(), d.color().b() );

      if( p==LightAblimient )
        set( d.ablimient().r(), d.ablimient().g(), d.ablimient().b() );
      }

    float operator [] ( int i ) const {
      return data[i];
      }

  private:
    std::string mname;
    float data[3];

    mutable Detail::ShInput sinput;

  friend class Device;
  };


template<>
class Uniform< Tempest::Texture2d > {
  public:
    Uniform():data(0){}

    void setName( const std::string& n ){
      mname = n;
      }

    const std::string& name() const {
      return mname;
      }
    void set( const Tempest::Texture2d * t ){
      data = t;
      }

    const Tempest::Texture2d* value() const {
      return data;
      }

  private:
    std::string mname;
    const Tempest::Texture2d* data;

    mutable Detail::ShInput sinput;

  friend class Device;
  };

}

#endif // UNIFORM_H
