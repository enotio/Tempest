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

    const Letter& letter(wchar_t ch , SpritesHolder &sp) const;

    void fetch(const std::u16string& str, SpritesHolder &sp) const;
    void fetch(const std::string& str, SpritesHolder &sp) const;
    Size textSize(const std::u16string& );
    Size textSize(const std::string& );

    int  size() const;

    void setBold( bool b );
    bool isBold() const;

    void setItalic( bool b );
    bool isItalic() const;

    void setSize( int s );
    LetterGeometry letterGeometry( wchar_t ch ) const;
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

      Letter* find( wchar_t c ) const;
      Letter& operator[]( wchar_t c );

      private:
        mutable Letter* l;
        mutable bool * e;
        mutable LMap*  n[256];
      };

    typedef LMap Leters;
    Leters * lt;

    //SpritesHolder* res;

    struct FreeTypeLib;
    static FreeTypeLib& ft();

    struct Key{
      //std::string name, baseName;
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
      } key;

    static std::vector< std::string > fnames;
    size_t findFontName( const std::string& n );
    size_t findFontNameTtf( const std::string& n,
                            const char* ttf  );

    static bool cmpS( const std::string &tg, const std::string& s, const char* ss);

    static std::map<Key, Leters*> letterBox;
    const Letter& fetchLeter(wchar_t ch, SpritesHolder &sp) const;
    const Letter& nullLeter (wchar_t ch ) const;

    void update();
  };

}

#endif // FONT_H
