#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <Tempest/Utility>
#include <Tempest/Event>
#include <Tempest/ImageCodec>
#include <Tempest/Platform>

namespace Tempest{

class Widget;
class Window;
class WindowOverlay;

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

    virtual Widget* addOverlay( WindowOverlay* ov ) = 0;

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
    static size_t peek(File* f, size_t skip, char* dest, size_t count );
    static size_t skip( File* f, size_t count );
    static bool   eof(File* f);
    static void   fclose( File* file );
    static size_t fsize( File *f );

    static bool loadImage( IDevice& file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char>& out );
    static bool saveImage( ODevice& file,
                           const ImageCodec::ImgInfo &info,
                           const std::vector<unsigned char>& in );

    static void processEvents(Tempest::Widget *w,
                               Event &e);

    static void emitEvent( Tempest::Window *w,
                           Event& e );
    static void emitEvent(Tempest::Window *w,
                           const KeyEvent &ebase,
                           const KeyEvent &scut,
                           Event::Type type );

    static void moveEvent( Tempest::Window *w, int cX, int cY);
    static void sizeEvent( Tempest::Window *w, int cW, int cH);
    static void setShowMode( Tempest::Window *w, int mode );
    static void activateEvent( Tempest::Window*w, bool a );

    enum GraphicsContexState{
      DestroyedByAndroid,
      Available,
      NotAvailable
      };
    virtual GraphicsContexState isGraphicsContextAvailable( Tempest::Window *w );

    static std::string    toUtf8 ( const std::u16string& str );
    static std::string    toUtf8 ( const char16_t* b, const char16_t* e );
    static std::u16string toUtf16( const std::string& str );
    static std::u16string toUtf16( const char* b, const char* e );

    static const std::string& androidActivityClass();
    static Event::KeyType translateKey( uint64_t scancode );
    static uint32_t translateChar( uint64_t scancode );

    template< class Img >
    void installImageCodec( Img* codec ){
      auto del = []( ImageCodec* c ){
        delete c;
        };
      installImageCodec(codec, del);
      }

    size_t imageCodecCount() const;
    ImageCodec& imageCodec( size_t id );

    struct CpuInfo{
      int cpuCount;
      };

    static CpuInfo cpuInfo();
  protected:
    SystemAPI();

    static Window* handle( Tempest::Window& );

    void addOverlay(Tempest::Window *w, WindowOverlay *ov );

    virtual File*  fopenImpl ( const char16_t* fname, const char* mode );
    virtual File*  fopenImpl ( const char* fname, const char* mode );
    virtual size_t readDataImpl ( File* f, char* dest, size_t count );
    virtual size_t writeDataImpl(File* f, const char* data, size_t count );
    virtual void   flushImpl(File* f );
    virtual size_t peekImpl (File* f, size_t skip, char* dest, size_t count );
    virtual size_t skipImpl(File* f, size_t count );
    virtual bool   eofImpl( File* file );
    virtual void   fcloseImpl( File* file );
    virtual size_t fsizeImpl( File* file );

    virtual Size implScreenSize() = 0;
    virtual bool testDisplaySettings( Window* w, const DisplaySettings& ) = 0;
    virtual bool setDisplaySettings ( Window* w, const DisplaySettings& ) = 0;

    virtual std::string       loadTextImpl( const char* file    );
    virtual std::string       loadTextImpl( const char16_t* file );

    virtual std::vector<char> loadBytesImpl( const char* file );
    virtual std::vector<char> loadBytesImpl( const char16_t* file );

    virtual bool writeBytesImpl(const char* file,     const std::vector<char> &f);
    virtual bool writeBytesImpl(const char16_t* file, const std::vector<char> &f);

    virtual bool loadImageImpl( Tempest::IDevice &imgBytes,
                                ImageCodec::ImgInfo &info,
                                std::vector<unsigned char>& out );

    virtual bool saveImageImpl( ODevice& file,
                                const ImageCodec::ImgInfo &info,
                                const std::vector<unsigned char>& out );

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

    std::vector<std::unique_ptr<ImageCodec, void (*)(ImageCodec* )>> codecs;

    void installImageCodec( ImageCodec* codec,
                            void (*del)(ImageCodec* ) );
    static void emitEventImpl( Tempest::Window *w,
                               Event& e );
  friend class AbstractAPI;
  };

}

#endif // ABSTRACTSYSTEMAPI_H
