#include "abstractholder.h"
#include <Tempest/Device>

using namespace Tempest;

AbstractHolderBase::AbstractHolderBase( Device &d ):m_device(d){
  device().addHolder( *this );
  }

AbstractHolderBase::~AbstractHolderBase(){
  device().delHolder( *this );
  }

Device &AbstractHolderBase::device() {
  return m_device;
  }

Device &AbstractHolderBase::device() const {
  return m_device;
  }
