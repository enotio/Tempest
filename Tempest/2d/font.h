#ifndef FONT_H
#define FONT_H

#include <string>
#include <Tempest/Sprite>

#include <unordered_map>
#include <map>
#include <memory>

namespace Tempest {

class Font{
  public:
    Font( const std::string& name, int sz );
    //Font( int sz );
    Font();

    struct LetterGeometry{
      Tempest::Size    size;
      Tempest::Point   dpos, advance;
      };

    struct Letter{
      Tempest::Sprite surf;
      Tempest::Size    size;
      Tempest::Point   dpos, advance;
      };

    const Letter& letter(char16_t ch, SpritesHolder &sp) const;

    void fetch(const std::u16string& str, SpritesHolder &sp) const;
    void fetch(const std::string& str, SpritesHolder &sp) const;

    Size textSize(const std::u16string& );
    Size textSize(const std::string& );

    Size textSize(const char16_t* b, const char16_t* e );
    Size textSize(const char* b, const char* e );

    Size textSize(const char16_t* str );
    Size textSize(const char* str );

    int  size() const;

    void setBold( bool b );
    bool isBold() const;

    void setItalic( bool b );
    bool isItalic() const;

    void setSize( int s );
    LetterGeometry letterGeometry( char16_t ch ) const;
  private:
    void init(const std::string &name, int sz);

    struct LMap{
      LMap(){
        l = 0;
        e = 0;
        std::fill( n, n+256, (LMap*)0 );
        }
      ~LMap(){
        delete l;
        delete e;
        for( int i=0; i<256; ++i )
          delete n[i];
        }

      Letter* find( char16_t c ) const;
      Letter& operator[]( char16_t c );

      private:
        mutable Letter* l;
        mutable bool * e;
        mutable LMap*  n[256];
      };

    struct Key{
      size_t name, baseName;
      int size;
      bool bold, italic;

      bool operator < ( const Key& other ) const{
        if( size < other.size )
          return 1;
        if( size > other.size )
          return 0;

        return name < other.name;
        }
      };

    typedef LMap Leters;
    Leters * lt;
    Key key;

    struct FreeTypeLib;
    static FreeTypeLib& ft();

    static std::vector< std::string > fnames;
    size_t findFontName( const std::string& n );
    size_t findFontNameTtf( const std::string& n,
                            const char* ttf  );

    static bool cmpS( const std::string &tg, const std::string& s, const char* ss);
    const Letter& fetchLeter(char16_t ch, SpritesHolder &sp) const;
    const Letter& nullLeter (char16_t ch ) const;

    void update();
  };

}

#endif // FONT_H
