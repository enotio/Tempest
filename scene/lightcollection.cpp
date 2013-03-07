#include "lightcollection.h"

const Tempest::LightCollection::DirectionLights&
    Tempest::LightCollection::direction() const {
  return dir;
  }

Tempest::LightCollection::DirectionLights &
    Tempest::LightCollection::direction() {
  return dir;
  }
