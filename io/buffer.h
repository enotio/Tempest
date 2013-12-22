#ifndef BUFFER_H
#define BUFFER_H

#include <Tempest/IODevice>
#include <vector>

namespace Tempest{

class BufferReader:public IDevice{
  public:
    BufferReader( const std::vector<char>& vec );

    size_t readData( char* dest, size_t count );
    size_t skip( size_t count );

    char peek() const;
    bool eof() const;
  private:
    const std::vector<char>& vec;
    size_t pos;
  };

class BufferWriter:public ODevice{
  public:
    BufferWriter( std::vector<char>& vec );

    size_t writeData(const char *src, size_t count );
  private:
    std::vector<char>& vec;
  };
}

#endif // BUFFER_H
