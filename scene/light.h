#ifndef LIGHT_H
#define LIGHT_H

#include <Tempest/Color>

namespace Tempest{

class DirectionLight{
  public:
    DirectionLight();

    void setDirection( double x, double y, double z );

    double xDirection() const;
    double yDirection() const;
    double zDirection() const;

    Color color() const;
    void setColor( const Color & c );


    Color ablimient() const;
    void setAblimient( const Color & c );
  private:
    double direction[3];
    Color cl, ao;
  };

}

#endif // LIGHT_H
