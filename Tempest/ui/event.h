#ifndef TEMPEST_EVENT_H
#define TEMPEST_EVENT_H

#include <Tempest/Utility>
#include <cstdint>

namespace Tempest{

class PainterDevice;

/**
 * \addtogroup UI_Event
 *  \ingroup GUI
 *  @{
 */

/*!
 * \brief The Event class is the base class of all event classes. Event objects contain event parameters.
 */
class Event {
  public:
    Event();
    virtual ~Event(){}

    void accept(){ accepted = true;  }
    void ignore(){ accepted = false; }

    bool isAccepted() const {
      return accepted;
      }

    enum Type {
      NoEvent = 0,
      MouseDown,
      MouseUp,
      MouseMove,
      MouseDrag,
      MouseWheel,
      MouseEnter,
      MouseLeave,
      KeyDown,
      KeyUp,
      Focus,
      Resize,
      Shortcut,
      Paint,
      Close,
      Gesture,

      Custom = 512
      };

    enum MouseButton {
      ButtonNone = 0,
      ButtonLeft,
      ButtonRight,
      ButtonMid,
      ButtonBack,
      ButtonForward
      };

    enum KeyType {
      K_NoKey = 0,
      K_ESCAPE,

      K_Control,
      K_Command, // APPLE command key

      K_Left,
      K_Up,
      K_Right,
      K_Down,

      K_Back,
      K_Tab,
      K_Delete,
      K_Insert,
      K_Return,
      K_Home,
      K_End,
      K_Pause,
      K_Shift,

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

    enum FocusReason {
      TabReason,
      ClickReason,
      WheelReason,
      UnknownReason
      };

    Type type () const{ return etype; }

  protected:
    void setType( Type t ){ etype = t; }

  private:
    bool accepted;
    Type etype;

  friend class SystemAPI;
  };

/*!
 * \brief The MouseEvent class contains event parameters for mouse and touch events.
 */
class MouseEvent : public Event {
  public:
    MouseEvent( int mx = -1, int my = -1,
                MouseButton b = ButtonNone,
                int mdelta = 0,
                int mouseID = 0,
                Type t = MouseMove )
      :x(mx), y(my), delta(mdelta), button(b), mouseID(mouseID){
      setType( t );
      }

    const int x, y, delta;
    const MouseButton button;

    const int mouseID;

    Point pos() const;
  };

class SizeEvent : public Event {
  public:
    SizeEvent( int w, int h ): w(w), h(h) {
      setType( Resize );
      }

    const int w, h;

    Size size() const;
  };

class KeyEvent: public Event {
  public:
    KeyEvent( KeyType  k = K_NoKey, Type t = KeyDown  ):key(k), u16(0){
      setType( t );
      }
    KeyEvent( uint32_t k, Type t = KeyDown ):key(K_NoKey), u16(k){
      setType( t );
      }
    KeyEvent( KeyType k, uint32_t k1, Type t = KeyDown ):key(k), u16(k1){
      setType( t );
      }

    const KeyType  key;
    const uint32_t u16;
  };

class FocusEvent: public Event {
  public:
    FocusEvent( bool in, FocusReason reason ): in(in), reason(reason) {
      setType( Focus );
      }

    const bool        in;
    const FocusReason reason;
  };

class PaintEvent: public Event {
  public:
    PaintEvent( PainterDevice & p, int ppass = 0 ):painter(p), pass(ppass){
      setType( Paint );
      }

    PainterDevice & painter;
    const unsigned int pass;

  private:
    using Event::accept;
  };

class CustomEvent: public Event {
  public:
    CustomEvent(){ setType(Custom); }
  };

class CloseEvent: public Event {
  public:
    CloseEvent(){ setType(Close); }
  };

class GestureRecognizer;
class AbstractGestureEvent: public Event {
  protected:
    AbstractGestureEvent( GestureRecognizer* owner );

  public:
    const Point& hotSpot() const;
    void setHotSpot( const Point& h );

    enum GestureType{
      gtNone    = 0,
      gtDragGesture,
      gtUser = 1024
      };

    GestureType gestureType() const;

    enum State{
      GestureStarted,
      GestureUpdated,
      GestureFinished,
      GestureCanceled
      };

    State state() const;
    GestureRecognizer& owner();
    const GestureRecognizer& owner() const;
  private:
    Point hs;
    GestureType gt;
    State st;

    GestureRecognizer* ow;

  protected:
    void setState( State s );
    void setGestureType( GestureType t );
  };

class DragGesture: public AbstractGestureEvent {
  public:
    DragGesture( GestureRecognizer* owner,
                 const Point& s,
                 const Point& p,
                 const Point& d,
                 State st ):AbstractGestureEvent(owner), start(s), pos(p), dpos(d){
      setGestureType(gtDragGesture);
      setState( st );
      setHotSpot(s);
      }

    const Point start, pos, dpos;
  };
/** @}*/
}

#endif // TEMPEST_EVENT_H
