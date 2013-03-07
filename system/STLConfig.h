#ifndef STLCONFIG_H
#define STLCONFIG_H

#ifdef __ANDROID__

namespace std{
  namespace tr1{}
  using namespace tr1;
  }

#endif

#endif // STLCONFIG_H
