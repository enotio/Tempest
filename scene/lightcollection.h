#ifndef LIGHTCOLLECTION_H
#define LIGHTCOLLECTION_H

#include <Tempest/AbstractLightCollection>
#include <vector>

namespace Tempest{

class LightCollection : public AbstractLightCollection {
  public:
    template< class L >
    class Pack: public AbstractLightCollection::Pack<L> {
      public:
        virtual int  size() const {
          return data.size();
          }

        virtual void resize( int s, const L& l = L() ) {
          data.resize(s, l);
          }

        virtual const L& operator [] ( int id ) const{
          return data[id];
          }

        virtual L& operator [] ( int id ) {
          return data[id];
          }

        virtual void push_back( const L& l ){
          return data.push_back(l);
          }

        virtual void pop_back(){
          return data.pop_back();
          }

        std::vector<L> data;
      };

    virtual const DirectionLights& direction() const;
    virtual DirectionLights& direction();
  private:
    Pack<DirectionLight> dir;
  };

}

#endif // LIGHTCOLLECTION_H
