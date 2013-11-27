#ifndef ANDROIDAPI_H
#define ANDROIDAPI_H

#include <Tempest/SystemAPI>

#ifdef __ANDROID__
#include <jni.h>

namespace Tempest{

class Window;

class AndroidAPI:public SystemAPI {
  public:
    static JavaVM *jvm();
    static JNIEnv *jenvi();
    static jclass appClass();

    static const char* internalStorage();
    static const char* externalStorage();

    static void toast( const std::string & s );
    static const std::string& iso3Locale();
  protected:
    AndroidAPI();
    ~AndroidAPI();

    bool testDisplaySettings( const DisplaySettings& );
    bool setDisplaySettings( const DisplaySettings& );
    Size implScreenSize();

    static bool startRender( Window* );
    static bool present( Window* );

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);
    int  nextEvents(bool &qiut);

    Window* createWindow(int w, int h);
    Window* createWindowMaximized();
    Window* createWindowMinimized();
    Window* createWindowFullScr();

    Point windowClientPos ( Window* );
    Size  windowClientRect( Window* );

    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

    std::string loadTextImpl( const char* file );
    std::string loadTextImpl( const wchar_t* file );

    std::vector<char> loadBytesImpl( const char* file );
    std::vector<char> loadBytesImpl( const wchar_t* file );
    
    bool loadImageImpl( const wchar_t* file,
                        ImageCodec::ImgInfo &info,
                        std::vector<unsigned char>& out );

    bool saveImageImpl( const wchar_t* file,
                        ImageCodec::ImgInfo &info,
                        std::vector<unsigned char>& in );

    bool isGraphicsContextAviable( Tempest::Window *w );
  public:
    friend class SystemAPI;
    friend class Opengl2x;

    template< class T >
    T loadAssetImpl( const char* file );

  };

}

#endif

#endif // ANDROIDAPI_H
