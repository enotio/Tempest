#include "programobject.h"

#include <Tempest/Device>

using namespace Tempest;

bool ProgramObject::isValid() const {
  return vs.isValid() && fs.isValid();
  }
