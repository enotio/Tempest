#ifndef LOG_H
#define LOG_H

#include <sstream>

namespace Tempest{

class Log {
  public:
    enum Mode{
      Info,
      Error,
      Debug
      };

    Log( Mode m = Log::Debug );
    ~Log();

    template< class T >
    Log &operator <<( const T& msg ) {
      st << msg;
      return *this;
      }

  private:
    std::stringstream st;
    void flush();
    Mode m;
  };
}

#endif // LOG_H
