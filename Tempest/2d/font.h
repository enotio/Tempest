#ifndef FONT_H
#define FONT_H

#include <string>
#include <Tempest/Sprite>

#include <unordered_map>
#include <map>
#include <memory>

namespace Tempest {

class FontElement{
  public:
    FontElement( const std::string& name, int sz );

    struct LetterGeometry{
      Tempest::Size    size;
      Tempest::Point   dpos, advance;
      };

    struct Letter{
      Tempest::Sprite surf;
      Tempest::Size   size;
      Tempest::Point  dpos, advance;
      };

    void fetch(const std::u16string& str, SpritesHolder &sp) const;
    void fetch(const std::string& str, SpritesHolder &sp) const;

    const Letter& letter(char16_t ch, SpritesHolder &sp) const;
    LetterGeometry letterGeometry( char16_t ch ) const;

    Size textSize(const std::u16string& );
    Size textSize(const std::string& );

    Size textSize(const char16_t* b, const char16_t* e );
    Size textSize(const char* b, const char* e );

    Size textSize(const char16_t* str );
    Size textSize(const char* str );

  private:
    FontElement();
    void init(const std::string &name, int sz);

    struct LMap{
      LMap();
      ~LMap();
      Letter* find( char16_t c ) const;
      Letter& operator[]( char16_t c );

      private:
        mutable Letter* let;
        mutable bool * e;
        mutable LMap*  n[256];
      };

    struct Key{
      size_t name;
      int    size;
      bool operator < ( const Key& other ) const;
      };

    typedef LMap Leters;
    Leters * lt;
    Key key;

    struct FreeTypeLib;
    static FreeTypeLib& ft();

    static std::vector< std::string > fnames;
    size_t findFontName( const std::string& n );

    static bool cmpS( const std::string &tg, const std::string& s, const char* ss);
    const Letter& fetchLeter(char16_t ch, SpritesHolder &sp) const;
    const Letter& nullLeter (char16_t ch ) const;

    void update();

  friend class Font;
  };

class Font{
  public:
    Font();
    Font( const std::string& name, int sz );
    Font( const FontElement& n,
          const FontElement& b,
          const FontElement& i,
          const FontElement& bi );

    typedef FontElement::LetterGeometry LetterGeometry;
    typedef FontElement::Letter         Letter;

    int  size() const;
    void setSize( int s );

    void setBold( bool b );
    bool isBold() const;

    void setItalic( bool b );
    bool isItalic() const;

    void fetch(const std::u16string& str, SpritesHolder &sp) const;
    void fetch(const std::string& str, SpritesHolder &sp) const;

    const Letter& letter(char16_t ch, SpritesHolder &sp) const;
    LetterGeometry letterGeometry( char16_t ch ) const;

    Size textSize(const std::u16string& );
    Size textSize(const std::string& );

    Size textSize(const char16_t* b, const char16_t* e );
    Size textSize(const char* b, const char* e );

    Size textSize(const char16_t* str );
    Size textSize(const char* str );

  private:
    uint8_t bold = 0, italic = 0;
    FontElement ttf[2][2];
  };

}

#endif // FONT_H
