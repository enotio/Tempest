#ifndef PIXMAP_H
#define PIXMAP_H

#include <Tempest/CopyWhenNeedPtr>
#include <vector>
#include <string>

namespace Tempest{

class Pixmap {
  public:
    Pixmap();
    Pixmap( const std::string& p );
    //Pixmap( const std::wstring& p );
    Pixmap( int w, int h, bool alpha );

    Pixmap( const Pixmap& p );
    Pixmap& operator = ( const Pixmap & p );

    bool load( const std::string & f );
    bool save( const std::string & f );

    inline int width() const { return mw; }
    inline int height() const { return mh; }

    struct Pixel{
      unsigned char r,g,b,a;

      unsigned char* data(){
        return &r;
        }

      const unsigned char* data() const {
        return &r;
        }

      template< class T >
      void assign( T it ){
        r = *it; ++it;
        g = *it; ++it;
        b = *it; ++it;
        a = *it; ++it;
        }
      };

    inline const unsigned char* const_data() const {
      return &data.const_value()->bytes[0];
      }

    void fill( const Pixel& p );

    inline Pixel at( int x, int y ) const {
      verifyFormatEditable();

      const unsigned char * p = &rawPtr[ (x + y*mw)*bpp ];
      Pixel r;

      r.r = p[0];
      r.g = p[1];
      r.b = p[2];

      if( bpp==4 )
        r.a = p[3]; else
        r.a = 255;

      return r;
      }

    inline void  set(int x, int y, const Pixel & p ) {
      verifyFormatEditable();

      if( true || !mrawPtr ){
        mrawPtr = &data.value()->bytes[0];
        rawPtr  = mrawPtr;
        }

      unsigned char * v = &mrawPtr[ (x + y*mw)*bpp ];

      v[0] = p.r;
      v[1] = p.g;
      v[2] = p.b;

      if( bpp==4 )
        v[3] = p.a;
      }

    bool  hasAlpha() const;
    void  addAlpha();

    enum Format{
      Format_RGB,
      Format_RGBA,
      Format_DXT1,
      Format_DXT3,
      Format_DXT5
      };

    Format format() const{ return frm; }
  private:
    struct Data{
      std::vector<unsigned char> bytes;
      // std::vector<unsigned char> dxt;
      };
    int mw, mh, bpp;
    Format      frm;

    struct DbgManip{
      typedef Data* T;

      struct Ref{
        Ref( const T& t ):data(t), count(1){}

        T   data;
        int count;
        };

      Ref * newRef(){
        return new Ref( T() );
        }

      Ref * newRef( const Ref * base );
      void delRef( Ref * r );

      bool isValid() const {
        return true;
        }
      };

    Detail::Ptr<Data*, DbgManip> data;
    const unsigned char * rawPtr;
    unsigned char * mrawPtr;

    void verifyFormatEditable() const {
      if( frm==Format_RGB || frm==Format_RGBA )
        return;

      Pixmap* p = const_cast<Pixmap*>(this);
      p->makeEditable();
      }

    void makeEditable();
  };

class PixEditor{
  public:
    PixEditor( Pixmap & out );

    void draw( int x, int y, const Pixmap &p );
    void copy( int x, int y, const Pixmap &p );
  private:
    Pixmap & out;
  };
}

#endif // PIXMAP_H
