#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <string>
#include <vector>

namespace Tempest{

class Window;
class MouseEvent;

class AbstractSystemAPI {
  public:
    virtual ~AbstractSystemAPI(){}

    struct Window;
    virtual Window* createWindow( int w, int h ) = 0;
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
