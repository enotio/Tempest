#ifndef GESTURERECOGNIZER_H
#define GESTURERECOGNIZER_H

namespace Tempest{

class Event;
class AbstractGestureEvent;

class GestureRecognizer {
  public:
    GestureRecognizer();
    virtual ~GestureRecognizer();

    virtual AbstractGestureEvent* event( const Event & );
  };

}

#endif // GESTURERECOGNIZER_H
