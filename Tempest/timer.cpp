#include "timer.h"

#include <Tempest/Application>

using namespace Tempest;

Timer::Timer():impl( new Data() ), timeout( impl->timeout ) {
  impl->minterval    = 0;//std::chrono::milliseconds::zero();
  impl->mrepeatCount = 4;
  }

Timer::~Timer() {
  if( impl->minterval != 0 )
    unreg();
  }

void Timer::start(uint64_t t) {
  if( impl->minterval == 0){
    if( t==0 )
      return;
    reg();
    }

  if( t==0 ){
    impl->minterval = 0;
    unreg();
    } else {
    impl->minterval = t;
    }

  impl->lastTimeout = Application::tickCount();
  }

void Timer::stop() {
  start(0);
  }

uint64_t Timer::interval() const {
  return impl->minterval;
  }

void Timer::setRepeatCount(uint64_t c) {
  impl->mrepeatCount = c;
  if( !impl->mrepeatCount )
    impl->mrepeatCount = 1;
  }

uint64_t Timer::repeatCount() const {
  return impl->mrepeatCount;
  }

void Timer::reg() {
  Application::app.timer.push_back(this);
  }

void Timer::unreg() {
  for( size_t i=0; i<Application::app.timer.size(); ++i )
    if( Application::app.timer[i]==this ){
      Application::app.timer[i] = 0;//Application::app.timer.back();
      //Application::app.timer.pop_back();
      return;
      }

  impl->minterval = 0;
  }
