#ifndef ATOMIC_H
#define ATOMIC_H

namespace Tempest{

namespace Detail{

struct Atomic {
  static void begin();
  static void end();
  };

}

}

#endif // ATOMIC_H
