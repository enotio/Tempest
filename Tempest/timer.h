#ifndef TIMER_H
#define TIMER_H

#include <Tempest/signal>
#include <cstdint>
#include <chrono>

namespace Tempest{

class Application;

class Timer {
  public:
    Timer();
    ~Timer();

    Timer( const Timer& )              = delete;
    Timer& operator = ( const Timer& ) = delete;

    signal<> timeout;

    void start( uint64_t t );
    void stop();
    uint64_t interval() const;

    void setRepeatCount( uint64_t c );
    uint64_t repeatCount() const;
  private:
    //std::chrono::milliseconds interval;
    //std::chrono::time_point<std::chrono::steady_clock> lastTimeout;

    uint64_t minterval;
    uint64_t lastTimeout;
    uint64_t mrepeatCount;

    void reg();
    void unreg();

  friend class Application;
  };

}

#endif // TIMER_H
