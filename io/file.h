#ifndef FILE_H
#define FILE_H

#include <Tempest/SystemAPI>
#include <Tempest/IODevice>

namespace Tempest{

class RFile : public IDevice {
  public:
    enum Mode {
      Default = 0,
      //Read   = 1,
      //Write  = 2,
      //Append = 4,
      Binary = 8
      };

    RFile( const char* name,     Mode m = Default );
    RFile( const char16_t* name, Mode m = Default );
    RFile( const RFile& )              = delete;
    RFile( RFile&& mov );
    RFile& operator = ( const RFile& ) = delete;
    RFile& operator = ( RFile&& mov );
    virtual ~RFile();

    size_t readData(char *dest, size_t count);
    size_t skip(size_t count);
    bool eof() const;
  private:
    SystemAPI::File* impl;
  };

class WFile : public ODevice {
  public:
    enum Mode {
      Default = 0,
      //Read   = 1,
      //Write  = 2,
      Append = 4,
      Binary = 8
      };

    WFile( const char* name,     Mode m = Default );
    WFile( const char16_t* name, Mode m = Default );
    WFile( WFile&& mov );
    WFile( const WFile& )              = delete;

    WFile& operator = ( WFile&& );
    WFile& operator = ( const WFile& ) = delete;
    virtual ~WFile();

    size_t writeData(const char *src, size_t count);
    void flush();
  private:
    SystemAPI::File* impl;
  };

}

#endif // FILE_H
