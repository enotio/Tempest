#include "timer.h"

#include <Tempest/Application>

using namespace Tempest;

Timer::Timer() {
  interval    = 0;//std::chrono::milliseconds::zero();
  mrepeatCount = 4;
  }

Timer::~Timer() {
  if( interval != 0 )
    unreg();
  }

void Timer::start(uint64_t t) {
  if( interval == 0){
    if( t==0 )
      return;
    reg();
    }

  if( t==0 ){
    interval = 0;
    unreg();
    } else {
    interval = t;
    }

  lastTimeout = Application::tickCount();
  }

void Timer::stop() {
  start(0);
  }

void Timer::setRepeatCount(uint64_t c) {
  mrepeatCount = c;
  if( !mrepeatCount )
    mrepeatCount = 1;
  }

uint64_t Timer::repeatCount() const {
  return mrepeatCount;
  }

void Timer::reg() {
  Application::app.timer.push_back(this);
  }

void Timer::unreg() {
  for( size_t i=0; i<Application::app.timer.size(); ++i )
    if( Application::app.timer[i]==this ){
      Application::app.timer[i] = Application::app.timer.back();
      Application::app.timer.pop_back();
      return;
      }

  interval = 0;
  }
