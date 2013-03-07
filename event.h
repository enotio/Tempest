#ifndef EVENT_H
#define EVENT_H

#include <Tempest/Utility>
#include <cstdint>

namespace Tempest{

class PainterDevice;

class Event {
  public:
    Event();
    virtual ~Event(){}

    void accept(){ accepted = true;  }
    void ignore(){ accepted = false; }

    bool isAccepted() const {
      return accepted;
      }


    enum MouseButton{
      ButtonNone = 0,
      ButtonLeft,
      ButtonRight,
      ButtonMid
      };

    enum KeyType{
      K_NoKey = 0,
      K_ESCAPE,

      K_Left,
      K_Up,
      K_Right,
      K_Down,

      K_Back,
      K_Delete,
      K_Return,

      K_F1,
      K_F2,
      K_F3,
      K_F4,
      K_F5,
      K_F6,
      K_F7,
      K_F8,
      K_F9,
      K_F10,
      K_F11,
      K_F12,
      K_F13,
      K_F14,
      K_F15,
      K_F16,
      K_F17,
      K_F18,
      K_F19,
      K_F20,
      K_F21,
      K_F22,
      K_F23,
      K_F24,

      K_A,
      K_B,
      K_C,
      K_D,
      K_E,
      K_F,
      K_G,
      K_H,
      K_I,
      K_J,
      K_K,
      K_L,
      K_M,
      K_N,
      K_O,
      K_P,
      K_Q,
      K_R,
      K_S,
      K_T,
      K_U,
      K_V,
      K_W,
      K_X,
      K_Y,
      K_Z,

      K_0,
      K_1,
      K_2,
      K_3,
      K_4,
      K_5,
      K_6,
      K_7,
      K_8,
      K_9,

      K_Last
      };

  private:
    bool accepted;
  };

class MouseEvent : public Event {
  public:
    MouseEvent( int mx = -1, int my = -1,
                MouseButton b = ButtonNone,
                int mdelta = 0 )
      :x(mx), y(my), delta(mdelta), button(b){}

    const int x, y, delta;
    const MouseButton button;

    Point pos() const;
  };

class SizeEvent : public Event {
  public:
    SizeEvent( int w, int h ): w(w), h(h){}

    const int w, h;

    Size size() const;
  };

class KeyEvent: public Event {
  public:
    KeyEvent( KeyType  k = K_NoKey ):key(k), u16(0){}
    KeyEvent( uint32_t k ):key(K_NoKey), u16(k){}
    KeyEvent( KeyType k, uint32_t k1 ):key(k), u16(k1){}

    const KeyType  key;
    const uint32_t u16;
  };

class PaintEvent: public Event {
  public:
    PaintEvent( PainterDevice & p, int ppass = 0 ):painter(p), pass(ppass){

      }

    PainterDevice & painter;
    const unsigned int pass;

  private:
    using Event::accept;
  };

class CustomEvent: public Event {
  public:
    CustomEvent(){}

  private:

  };

}

#endif // EVENT_H
