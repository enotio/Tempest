#include "devicesm5.h"

using namespace Tempest;

DeviceSM5::DeviceSM5(const AbstractAPI &dx, void *hwnd)
  :Device(dx, hwnd) {
  }

DeviceSM5::DeviceSM5(const AbstractAPI &dx, const Device::Options &opt, void *hwnd)
  :Device(dx, opt, hwnd) {
  }

void DeviceSM5::deleteShader( GraphicsSubsystem::TessShader * s ) const {
  GraphicsSubsystem::DeleteEvent e;
  e.ts = s;
  event(e);
  shadingLang().deleteShader(s);
  }

void DeviceSM5::deleteShader( GraphicsSubsystem::EvalShader *s  ) const {
  GraphicsSubsystem::DeleteEvent e;
  e.es = s;
  event(e);
  shadingLang().deleteShader(s);
  }

void DeviceSM5::bind(const TessShader &ts) {
  shadingLang().bind(ts);
  }

void DeviceSM5::bind(const EvalShader &es) {
  shadingLang().bind(es);
  }

void DeviceSM5::bindShaders(const TessShader &ts, const EvalShader &es) {
  bind(ts);
  bind(es);
  }
