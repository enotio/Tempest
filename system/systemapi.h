#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <cstdint>
#include <string>
#include <vector>

#include <Tempest/Utility>
#include <Tempest/Event>

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
    static std::string loadText( const std::wstring& file );

    static std::string loadText( const char* file );
    static std::string loadText( const wchar_t* file );

    static std::vector<char> loadBytes( const char* file );
    static std::vector<char> loadBytes( const wchar_t* file );
    static bool writeBytes(const wchar_t* file , const std::vector<char> &f);

    static bool loadImage( const wchar_t* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& out );
    static bool saveImage( const wchar_t* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& in );

    static bool loadImage( const char* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& out );
    static bool saveImage( const char* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& in );

    static void processEvents( Tempest::Widget *w,
                               MouseEvent& e,
                               Event::Type type );
    static void processEvents( Tempest::Widget *w,
                               KeyEvent &e,
                               Event::Type type );
    static void processEvents( Tempest::Widget *w,
                               CloseEvent &e,
                               Event::Type type );

    static void mkMouseEvent( Tempest::Window *w,
                              MouseEvent& e,
                              Event::Type type );

    static void mkKeyEvent( Tempest::Window *w,
                            KeyEvent& e,
                            Event::Type type );

    static void mkCloseEvent( Tempest::Window *w,
                              CloseEvent& e,
                              Event::Type type );

    static void sizeEvent( Tempest::Window *w, int cW, int cH);
    static void activateEvent( Tempest::Window*w, bool a );

    virtual bool isGraphicsContextAviable( Tempest::Window *w );

    static std::string  toUtf8   (const std::wstring& str);
    static std::wstring toWstring( const std::string& str );

    static const std::string& androidActivityClass();
    static Event::KeyType translateKey( uint64_t scancode );
  protected:
    SystemAPI(){}

    virtual Size implScreenSize() = 0;
    virtual bool testDisplaySettings( const DisplaySettings& ) = 0;
    virtual bool setDisplaySettings ( const DisplaySettings& ) = 0;

    virtual std::string       loadTextImpl( const char* file    ) = 0;
    virtual std::string       loadTextImpl( const wchar_t* file ) = 0;

    virtual std::vector<char> loadBytesImpl( const char* file ) = 0;
    virtual std::vector<char> loadBytesImpl( const wchar_t* file ) = 0;

    virtual bool writeBytesImpl(const wchar_t* file , const std::vector<char> &f);

    virtual bool loadImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out );
    virtual bool saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out );

    virtual bool loadImageImpl( const wchar_t* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;
    virtual bool saveImageImpl( const wchar_t* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;

    virtual bool loadS3TCImpl(const std::vector<char> &data,
                               int &w,
                               int &h,
                               int &bpp,
                               std::vector<unsigned char> &out );

    virtual bool loadPngImpl(const std::vector<char> &data,
                             int &w,
                             int &h,
                             int &bpp,
                             std::vector<unsigned char> &out );

    virtual const std::string& androidActivityClassImpl();

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

  friend class AbstractAPI;
  };

}

#endif // ABSTRACTSYSTEMAPI_H
