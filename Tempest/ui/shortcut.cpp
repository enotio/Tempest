#include "shortcut.h"

#include <Tempest/Widget>
#include <algorithm>
#include <Tempest/Assert>

using namespace Tempest;

Shortcut::Shortcut() {
  m.lkey    = 0;
  m.key     = KeyEvent::K_NoKey;
  m.modyfer = KeyEvent::K_NoKey;
  m.owner   = 0;
  }

Shortcut::Shortcut( Widget *w,
                    KeyEvent::KeyType k,
                    Event::KeyType md ) {
  T_ASSERT(w!=0);

  m.lkey    = 0;
  m.key     = k;
  m.modyfer = md;
  m.owner   = w;

  w->skuts.push_back(this);
  }

Shortcut::Shortcut(const Shortcut &sc) {
  m = sc.m;
  m.owner->skuts.push_back(this);
  }

Shortcut::~Shortcut() {
  if( m.owner )
    m.owner->skuts.resize( std::remove( m.owner->skuts.begin(), m.owner->skuts.end(), this ) -
                           m.owner->skuts.begin() );
  }

Shortcut &Shortcut::operator =( const Shortcut &sc ) {
  if( m.owner )
    m.owner->skuts.resize( std::remove( m.owner->skuts.begin(), m.owner->skuts.end(), this ) -
                           m.owner->skuts.begin() );

  m = sc.m;

  if( m.owner )
    m.owner->skuts.push_back(this);

  activated = sc.activated;

  return *this;
  }

void Shortcut::setKey( uint32_t k ) {
  m.lkey = k;
  }

uint32_t Shortcut::lkey() const {
  return m.lkey;
  }

void Shortcut::setKey( Event::KeyType k ) {
  m.key = k;
  }

Event::KeyType Shortcut::key() const {
  return m.key;
  }
