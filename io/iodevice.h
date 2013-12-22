#ifndef IODEVICE_H
#define IODEVICE_H

#include <cstddef>

namespace Tempest{

class AbstractIODevice{
  public:
    virtual ~AbstractIODevice(){}
  };

class IDevice: public AbstractIODevice {
  public:
    virtual size_t readData( char* dest, size_t count ) = 0;
    virtual size_t skip( size_t count ) = 0;

    virtual char peek() const = 0;
    virtual bool eof()  const = 0;
  };

class ODevice: public AbstractIODevice {
  public:
    virtual size_t writeData( const char* src, size_t count ) = 0;
    virtual void flush();
  };


class MemReader:public IDevice{
  public:
    MemReader( const char* vec, size_t sz );
    MemReader( const unsigned char* vec, size_t sz );
    size_t readData( char* dest, size_t count );
    size_t skip( size_t count );

    char peek() const;
    bool eof()  const;

  private:
    const char* vec;
    size_t sz;
  };

class MemWriter:public IDevice{
  public:
    MemWriter( char* vec, size_t sz );
    MemWriter( unsigned char* vec, size_t sz );
    size_t writeData( char* src, size_t count );

  private:
    char* vec;
    size_t sz;
  };
}

#endif // IODEVICE_H
