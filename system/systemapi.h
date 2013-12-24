#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <Tempest/Utility>
#include <Tempest/Event>
#include <Tempest/ImageCodec>

namespace Tempest{

class Widget;
class Window;

class DisplaySettings;

class SystemAPI{
  public:
    virtual ~SystemAPI(){}

    struct Window;
    virtual Window* createWindow( int w, int h ) = 0;
    virtual Window* createWindowMaximized() = 0;
    virtual Window* createWindowMinimized() = 0;
    virtual Window* createWindowFullScr()   = 0;

    virtual Point   windowClientPos ( Window* )   = 0;
    virtual Size    windowClientRect( Window* )   = 0;

    virtual void deleteWindow( Window* ) = 0;
    virtual void setGeometry( Window*, int x, int y, int w, int h ) = 0;
    virtual void show( Window* ) = 0;
    virtual void bind( Window*, Tempest::Window* ) = 0;

    static Size screenSize();

    static SystemAPI& instance();

    struct ApplicationInitArgs;
    virtual void startApplication( ApplicationInitArgs* ) = 0;
    virtual void endApplication() = 0;
    virtual int  nextEvent(bool &qiut) = 0;
    virtual int  nextEvents(bool &qiut) = 0;

    static std::string loadText( const std::string& file );
    static std::string loadText( const std::u16string& file );

    static std::string loadText( const char* file );
    static std::string loadText( const char16_t* file );

    static std::vector<char> loadBytes( const char* file );
    static std::vector<char> loadBytes( const char16_t *file );
    static bool writeBytes(const char* file,     const std::vector<char> &f);
    static bool writeBytes(const char16_t* file, const std::vector<char> &f);

    class  File;
    static File*  fopen( const char16_t* fname, const char* mode );
    static File*  fopen( const char* fname, const char* mode );
    static size_t readData ( File* f, char* dest, size_t count );
    static size_t writeData( File* f, const char* src, size_t count );
    static void   flush(File* f);
    static size_t skip( File* f, size_t count );
    static bool   eof(File* f);
    static void   fclose( File* file );

    static bool loadImage( const char16_t* file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char>& out );
    static bool saveImage( const char16_t* file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char>& in );

    static bool loadImage( const char* file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char>& out );
    static bool saveImage( const char* file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char>& in );

    static void processEvents( Tempest::Widget *w,
                               AbstractGestureEvent &e,
                               Event::Type type);
    static void processEvents( Tempest::Widget *w,
                               MouseEvent& e,
                               Event::Type type );
    static void processEvents( Tempest::Widget *w,
                               KeyEvent &e,
                               Event::Type type );
    static void processEvents( Tempest::Widget *w,
                               CloseEvent &e,
                               Event::Type type );

    static void emitEvent( Tempest::Window *w,
                           MouseEvent& e,
                           Event::Type type );

    static void emitEvent( Tempest::Window *w,
                           KeyEvent& e,
                           Event::Type type );

    static void emitEvent( Tempest::Window *w,
                           CloseEvent& e,
                           Event::Type type );

    static void moveEvent( Tempest::Window *w, int cX, int cY);
    static void sizeEvent( Tempest::Window *w, int cW, int cH);
    static void setShowMode( Tempest::Window *w, int mode );
    static void activateEvent( Tempest::Window*w, bool a );

    enum GraphicsContexState{
      DestroyedByAndroid,
      Aviable,
      NotAviable
      };
    virtual GraphicsContexState isGraphicsContextAviable( Tempest::Window *w );

    static std::string    toUtf8 (const std::u16string& str);
    static std::u16string toUtf16( const std::string& str );

    static const std::string& androidActivityClass();
    static Event::KeyType translateKey( uint64_t scancode );

    void installImageCodec( ImageCodec* codec );
    size_t imageCodecCount() const;
    ImageCodec& imageCodec( size_t id );

    struct CpuInfo{
      int cpuCount;
      };

    static CpuInfo cpuInfo();
  protected:
    SystemAPI();

    virtual File*  fopenImpl ( const char16_t* fname, const char* mode );
    virtual File*  fopenImpl ( const char* fname, const char* mode );
    virtual size_t readDataImpl ( File* f, char* dest, size_t count );
    virtual size_t writeDataImpl(File* f, const char* data, size_t count );
    virtual void   flushImpl(File* f );
    virtual size_t skipImpl(File* f, size_t count );
    virtual bool   eofImpl( File* file );
    virtual void   fcloseImpl( File* file );

    virtual Size implScreenSize() = 0;
    virtual bool testDisplaySettings( const DisplaySettings& ) = 0;
    virtual bool setDisplaySettings ( const DisplaySettings& ) = 0;

    virtual std::string       loadTextImpl( const char* file    ) = 0;
    virtual std::string       loadTextImpl( const char16_t* file ) = 0;

    virtual std::vector<char> loadBytesImpl( const char* file ) = 0;
    virtual std::vector<char> loadBytesImpl( const char16_t* file ) = 0;

    virtual bool writeBytesImpl(const char* file,     const std::vector<char> &f);
    virtual bool writeBytesImpl(const char16_t* file, const std::vector<char> &f);

    virtual bool loadImageImpl( const char* file,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );

    virtual bool loadImageImpl( const char16_t* file,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );
    virtual bool loadImageImpl( const std::vector<char>& imgBytes,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );

    virtual bool saveImageImpl( const char* file,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );
    virtual bool saveImageImpl( const char16_t* file,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );

    virtual const std::string& androidActivityClassImpl();
    virtual CpuInfo cpuInfoImpl() = 0;

    struct TranslateKeyPair{
      uint64_t       src;
      Event::KeyType result;
      };

    void setupKeyTranslate( const TranslateKeyPair k[] );
    void setFuncKeysCount( int c );    
  private:
    SystemAPI( const SystemAPI& ){}
    SystemAPI& operator = ( const SystemAPI&){return *this;}

    struct KeyInf{
      KeyInf():fkeysCount(1){}

      std::vector<TranslateKeyPair> keys;
      std::vector<TranslateKeyPair> a, k0, f1;
      int fkeysCount;
      };

    KeyInf ki;
    struct GestureDeleter;

    std::vector<std::unique_ptr<ImageCodec>> codecs;
    Detail::Spin  byteBuffer;
    std::vector<char> buffer;

  friend class AbstractAPI;
  };

}

#endif // ABSTRACTSYSTEMAPI_H
