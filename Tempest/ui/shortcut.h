#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <Tempest/Event>
#include <Tempest/signal>

namespace Tempest{

class Widget;

class Shortcut {
  public:
    Shortcut();

    Shortcut( Widget * w,
              KeyEvent::KeyType key,
              KeyEvent::KeyType md   = KeyEvent::K_NoKey );
    Shortcut( const Shortcut& sc );
    ~Shortcut();

    Shortcut& operator = ( const Shortcut& sc );

    void setKey( KeyEvent::KeyType key );
    void setKey( uint32_t key );
    void setModifier( KeyEvent::KeyType key );

    KeyEvent::KeyType key()  const;
    uint32_t          lkey() const;
    KeyEvent::KeyType modifier()  const;

    signal<> activated;

  private:
    struct M{
      KeyEvent::KeyType key, modifier;
      uint32_t lkey;
      Widget * owner;
      } m;

  friend class Widget;
  };

}

#endif // SHORTCUT_H
