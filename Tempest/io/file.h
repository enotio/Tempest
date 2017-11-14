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
      //Binary = 8
      };

    RFile( const char* name,     Mode m = Default );
    RFile( const char16_t* name, Mode m = Default );

    RFile( const std::string&    name, Mode m = Default );
    RFile( const std::u16string& name, Mode m = Default );

    RFile( RFile&& mov );
    virtual ~RFile();

    RFile& operator = ( RFile&& mov );

    size_t readData(char *dest, size_t count);
    void   skip(size_t count);
    size_t peek( size_t skip, char* dest, size_t maxSize ) const;

    bool eof()    const;
    bool isOpen() const;
    size_t size() const;
  private:
    SystemAPI::File* impl;
    RFile(const RFile&) = delete;
    RFile& operator = (const RFile&) = delete;
  };

class WFile : public ODevice {
  public:
    enum Mode {
      Default = 0,
      //Read   = 1,
      //Write  = 2,
      Append = 4,
      //Binary = 8
      };

    WFile( const char* name,     Mode m = Default );
    WFile( const char16_t* name, Mode m = Default );

    WFile( const std::string&    name, Mode m = Default );
    WFile( const std::u16string& name, Mode m = Default );

    WFile( WFile&& mov );
    virtual ~WFile();

    WFile& operator = ( WFile&& );

    size_t writeData(const char *src, size_t count);
    void flush();

    bool isOpen() const;
  private:
    SystemAPI::File* impl;

    WFile& operator = (const WFile&) = delete;
    WFile(const WFile&) = delete;
  };

}

#endif // FILE_H
