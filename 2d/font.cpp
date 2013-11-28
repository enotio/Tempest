#include "font.h"

#include <Tempest/Pixmap>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

struct Tempest::Font::FreeTypeLib{
  FreeTypeLib(){
    FT_Init_FreeType( &library );
    Tempest::Font::fnames.reserve(16);
    }

  ~FreeTypeLib(){
    FT_Done_FreeType( library );

    std::map<Tempest::Font::Key, Tempest::Font::Leters*>::iterator
        b, e;
    b = Tempest::Font::letterBox.begin();
    e = Tempest::Font::letterBox.end();

    for( ; b!=e; ++b )
      delete b->second;
    }

  FT_Library    library;
  };

std::map<Tempest::Font::Key, Tempest::Font::Leters*>
  Tempest::Font::letterBox;

std::vector< std::string > Tempest::Font::fnames;

Tempest::Font::FreeTypeLib& Tempest::Font::ft(){
  static FreeTypeLib lib;
  return lib;
  }

size_t Tempest::Font::findFontNameTtf( const std::string &ft,
                                       const char* ttf ) {
  for( size_t i=0; i<fnames.size(); ++i ){
    if( cmpS( fnames[i], ft, ttf ) )
      return i;
    }

  fnames.push_back(ft+ttf);
  return fnames.size()-1;
  }

size_t Tempest::Font::findFontName(const std::string &n) {
  for( size_t i=0; i<fnames.size(); ++i )
    if( fnames[i]==n )
      return i;

  fnames.push_back(n);
  return fnames.size()-1;
  }

bool Tempest::Font::cmpS( const std::string &tg,
                          const std::string& s,
                          const char* ss ){
  if( tg.size()<s.size() )
    return 0;

  for( size_t i=0; i<s.size(); ++i )
    if( tg[i]!=s[i] )
      return 0;

  for( size_t i=0; ss[i]; ++i )
    if( !( s.size()+i<=tg.size() && tg[i+s.size()]==ss[i] ) )
      return 0;

  return 1;
  }

Tempest::Font::Font( const std::string &name,
                     int sz) {
  init(name, sz);
  }

Tempest::Font::Font(int sz) {
#ifdef __ANDROID__
  init("/system/fonts/DroidSans", sz);
#else
  init("./data/arial", sz);
#endif
  }

Tempest::Font::Font() {
#ifdef __ANDROID__
  init("/system/fonts/DroidSans", 16);
#else
  init("./data/arial", 16);
#endif
  }

void Tempest::Font::init( const std::string& name, int sz ) {
  key.name     = findFontNameTtf(name, ".ttf");
  key.baseName = findFontName(name);

  key.size = sz;

  key.bold   = false;
  key.italic = false;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

const Tempest::Font::Letter&
    Tempest::Font::fetchLeter( wchar_t ch,
                               Tempest::SpritesHolder & res  ) const {
  Leters & leters = *lt;

  if( Letter *l = leters.find(ch) ){
    return *l;
    }

  FT_Face       face;
  FT_Vector     pen = {0,0};
  FT_Error err = 0;

  err = FT_New_Face( ft().library, fnames[key.name].c_str(), 0, &face );
  if( err )
    return nullLeter(ch);

  err = FT_Set_Pixel_Sizes( face, key.size, key.size );
  if( err )
    return nullLeter(ch);

  FT_Set_Transform( face, 0, &pen );

  Letter letter;
  if( FT_Load_Char( face, ch, FT_LOAD_RENDER ) ){
    Letter &ref = leters[ch];
    ref = letter;
    //ref.surf.data.tex = 0;
    return ref;
    }

  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap& bmap = slot->bitmap;

  letter.dpos = Tempest::Point( slot->bitmap_left,
                                key.size - slot->bitmap_top );

  if( bmap.width!=0 && bmap.rows!=0 ){
    Tempest::Pixmap pixmap( bmap.width, bmap.rows, true );

    for( int i=0; i<pixmap.width(); ++i )
      for( int r=0; r<pixmap.height(); ++r ){
        uint8_t lum = bmap.buffer[r * bmap.width + i];
        Tempest::Pixmap::Pixel p = {255, 255, 255, lum};
        pixmap.set( i,r, p );
        }

    //pixmap.save("./l.png");
    letter.surf      = res.load( pixmap );
    }

  letter.size      = Tempest::Size( bmap.width, bmap.rows );
  letter.advance   = Tempest::Point( slot->advance.x >> 6,
                                     slot->advance.y >> 6 );

  FT_Done_Face( face );

  Letter &ref = leters[ch];
  ref = letter;
  return ref;
  }

Tempest::Font::LetterGeometry Tempest::Font::letterGeometry( wchar_t ch ) const {
  Leters & leters = *lt;

  if( Letter *l = leters.find(ch) ){
    LetterGeometry r;
    r.advance = l->advance;
    r.dpos    = l->dpos;
    r.size    = l->size;
    return r;
    }

  FT_Face       face;
  FT_Vector     pen = {0,0};
  FT_Error err = 0;

  err = FT_New_Face( ft().library, fnames[key.name].c_str(), 0, &face );
  if( err )
    return LetterGeometry();

  err = FT_Set_Pixel_Sizes( face, key.size, key.size );
  if( err ){
    return LetterGeometry();
    }

  FT_Set_Transform( face, 0, &pen );

  LetterGeometry letter;
  if( FT_Load_Char( face, ch, FT_LOAD_RENDER ) ){
    return letter;
    }

  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap& bmap = slot->bitmap;

  letter.dpos = Tempest::Point( slot->bitmap_left,
                                 key.size - slot->bitmap_top );

  letter.size      = Tempest::Size( bmap.width, bmap.rows );
  letter.advance   = Tempest::Point( slot->advance.x >> 6,
                                     slot->advance.y >> 6 );

  FT_Done_Face( face );
  return letter;
  }

const Tempest::Font::Letter &
  Tempest::Font::nullLeter(wchar_t ch) const {
  Leters & leters = *lt;
  Letter letter;

  Letter &ref = leters[ch];
  ref = letter;
  //ref.surf.data.tex = 0;
  return ref;
  }

void Tempest::Font::fetch( const std::wstring &str,
                           Tempest::SpritesHolder & sp  ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( str[i], sp );
  }

void Tempest::Font::fetch( const std::string &str,
                           Tempest::SpritesHolder & sp ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( str[i], sp );
  }

Tempest::Size Tempest::Font::textSize( const std::wstring & str ) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const LetterGeometry& l = letterGeometry( str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw,th);
  }

Tempest::Size Tempest::Font::textSize( const std::string & str ) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    LetterGeometry l = letterGeometry( str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tw = std::max( tx+l.advance.x, tw );
    th = std::max( ty+l.advance.y, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw,th);
  }

int Tempest::Font::size() const {
  return key.size;
  }

void Tempest::Font::setBold(bool b) {
  key.bold = b;
  update();
  }

bool Tempest::Font::isBold() const {
  return key.bold;
  }

void Tempest::Font::setItalic(bool b) {
  key.italic = b;
  update();
  }

bool Tempest::Font::isItalic() const {
  return key.italic;
  }

void Tempest::Font::setSize(int s) {
  key.size = s;
  update();
  }

const Tempest::Font::Letter&
  Tempest::Font::letter( wchar_t ch,
                         Tempest::SpritesHolder & sp ) const {
  const Tempest::Font::Letter& tmp = fetchLeter(ch,sp);
  sp.flush();
  return tmp;
  }

void Tempest::Font::update() {
  key.name = key.baseName;

  if( key.bold && key.italic )
    key.name =  findFontNameTtf( fnames[key.baseName], "bi.ttf"); else
  if( key.bold )
    key.name =  findFontNameTtf( fnames[key.baseName], "b.ttf"); else
  if( key.italic )
    key.name =  findFontNameTtf( fnames[key.baseName], "i.ttf"); else
    key.name =  findFontNameTtf( fnames[key.baseName], ".ttf");

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

Tempest::Font::Letter *Tempest::Font::LMap::find(wchar_t c) const {
  unsigned char cp[sizeof(c)];
  for( size_t i=0; i<sizeof(wchar_t); ++i){
    cp[i] = c%256;
    c/=256;
    }

  const LMap *l = this;

  for( size_t i=sizeof(wchar_t)-1; i>0; --i ){
    unsigned char cx = cp[i];
    if( l->n[cx]==0 )
      return 0;

    l = l->n[cx];
    }

  if( l->l==0 ){
    l->l = new Letter[256];
    l->e = new bool[256];
    std::fill( l->e, l->e+256, false );
    }

  if( l->e[cp[0]] )
    return l->l+cp[0];

  return 0;
  }

Tempest::Font::Letter &Tempest::Font::LMap::operator [](wchar_t c) {
  unsigned char cp[sizeof(c)];
  for( size_t i=0; i<sizeof(wchar_t); ++i){
    cp[i] = c%256;
    c/=256;
    }

  const LMap *l = this;

  for( size_t i=sizeof(wchar_t)-1; i>0; --i ){
    unsigned char cx = cp[i];
    if( l->n[cx]==0 ){
      l->n[cx] = new LMap();
      }

    l = l->n[cx];
    }

  if( l->l==0 ){
    l->l = new Letter[256];
    l->e = new bool[256];
    std::fill( l->e, l->e+256, false );
    }

  l->e[cp[0]] = 1;
  return *(l->l+cp[0]);
  }
