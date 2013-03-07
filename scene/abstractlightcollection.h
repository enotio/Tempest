#ifndef ABSTRACTLIGHTCOLLECTION_H
#define ABSTRACTLIGHTCOLLECTION_H

#include <Tempest/Light>

namespace Tempest{

class AbstractLightCollection {
  public:
    virtual ~AbstractLightCollection(){}

    template< class L >
    class Pack{
      public:
        virtual ~Pack(){}

        virtual int  size() const = 0;
        virtual void resize( int s, const L& = L() ) = 0;

        virtual const L& operator [] ( int id ) const = 0;
        virtual L& operator [] ( int id ) = 0;

        virtual void push_back( const L& l ) = 0;
        virtual void pop_back() = 0;
      };
    typedef Pack<DirectionLight> DirectionLights;

    virtual const DirectionLights& direction() const = 0;
    virtual DirectionLights& direction() = 0;
  };

}

#endif // ABSTRACTLIGHTCOLLECTION_H
