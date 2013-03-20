#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <string>
#include <vector>

#include <Tempest/Utility>

namespace Tempest{

class Window;
class MouseEvent;
class SizeEvent;

class AbstractSystemAPI {
  public:
    virtual ~AbstractSystemAPI(){}

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

    static AbstractSystemAPI& instance();

    struct ApplicationInitArgs;
    virtual void startApplication( ApplicationInitArgs* ) = 0;
    virtual void endApplication() = 0;
    virtual int  nextEvent(bool &qiut) = 0;

    static std::string loadText( const char* file );
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

    static void mkMouseEvent( Tempest::Window *w,
                              MouseEvent& e,
                              int type );

    static void sizeEvent(Tempest::Window *w,
                           int winW , int winH, int cW, int cH);
    static void activateEvent( Tempest::Window*w, bool a );
  protected:
    AbstractSystemAPI(){}

    virtual std::string loadTextImpl( const char* file ) = 0;
    virtual bool loadImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;
    virtual bool saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;

  private:
    AbstractSystemAPI( const AbstractSystemAPI& ){}
    AbstractSystemAPI& operator = ( const AbstractSystemAPI&){return *this;}
  };

}

#endif // ABSTRACTSYSTEMAPI_H
