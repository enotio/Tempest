#include "font.h"

#include <Tempest/Pixmap>
#include <Tempest/File>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include <algorithm>

struct Tempest::FontElement::FreeTypeLib{
  FreeTypeLib(){
    FT_Init_FreeType( &library );
    Tempest::FontElement::fnames.reserve(16);
    }

  ~FreeTypeLib(){
    std::map<Tempest::FontElement::Key, Tempest::FontElement::Leters*>::iterator
        b, e;
    b = letterBox.begin();
    e = letterBox.end();

    for( ; b!=e; ++b )
      delete b->second;

    FT_Done_FreeType( library );
    }

  std::map<Key, Leters*> letterBox;
  FT_Library    library;

  static unsigned long ft_stream_io( FT_Stream      stream,
                                     unsigned long  offset,
                                     unsigned char* buffer,
                                     unsigned long  count ) {
    if( !count && offset > stream->size )
      return 1;

    MemReader* file;
    file = ( (MemReader*)stream->descriptor.pointer );
    return file->peek(offset, (char*)buffer, count );
    }

  static void ft_stream_close( FT_Stream  /*stream*/ ) {
    return;
    }

  void mkStream( FT_StreamRec& stream, MemReader& file ){
    stream.descriptor.pointer = &file;
    stream.read  = ft_stream_io;
    stream.close = ft_stream_close;
    stream.size  = -1;
    }

  FT_Error New_Face( FT_Library    library,
                     FT_StreamRec& stream,
                     FT_Long       face_index,
                     FT_Face      *aface ) {
    FT_Open_Args  args;

    args.pathname = 0;

    args.flags    = FT_OPEN_STREAM;
    args.stream   = &stream;

    return FT_Open_Face( library, &args, face_index, aface );
    }
  };

std::vector< std::string > Tempest::FontElement::fnames;
std::vector< std::unique_ptr<char[]> > Tempest::FontElement::fdata;

Tempest::FontElement::FreeTypeLib& Tempest::FontElement::ft(){
  static FreeTypeLib lib;
  return lib;
  }

size_t Tempest::FontElement::findFontName(const std::string &n) {
  for( size_t i=0; i<fnames.size(); ++i )
    if( fnames[i]==n )
      return i;

  fnames.push_back(n);
  fdata.resize(fnames.size());
  return fnames.size()-1;
  }

Tempest::FontElement::FontElement( const std::string &name,
                                   int sz) {
  init(name, sz);
  }

Tempest::FontElement::FontElement() {
#ifdef __ANDROID__
  init("/system/fonts/DroidSans", 16);
#else
  init("./data/arial", 16);
#endif
  }

void Tempest::FontElement::init( const std::string& name, int sz ) {
  key.name = findFontName(name);
  key.size = sz;

  lt = ft().letterBox[key];
  if( !lt ){
    lt = new Leters();
    ft().letterBox[key] = lt;
    }
  }

const Tempest::FontElement::Letter&
    Tempest::FontElement::fetchLeter( char16_t ch,
                                      Tempest::SpritesHolder & res  ) const {
  Leters & leters = *lt;

  if( Letter *l = leters.find(ch) ){
    return *l;
    }

  FT_Face   face = 0;
  FT_Vector pen  = {0,0};
  FT_Error  err  = 0;

  if(fdata[key.name]==nullptr){
    RFile file(fnames[key.name].c_str());
    size_t sz = file.size();
    char * ch = new char[sz];
    fdata[key.name].reset(ch);
    if(file.readData(ch,sz)!=sz)
      fdata[key.name].reset();
    }
  Tempest::MemReader reader(fdata[key.name].get(),-1);
  FT_StreamRec stream;
  ft().mkStream(stream,reader);

  err = ft().New_Face( ft().library, stream, 0, &face );
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
    return ref;
    }
  
  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap& bmap   = slot->bitmap;

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
    letter.surf = res.load( pixmap );
    }

  letter.size      = Tempest::Size( bmap.width, bmap.rows );
  letter.advance   = Tempest::Point( slot->advance.x >> 6,
                                     slot->advance.y >> 6 );

  FT_Done_Face( face );

  Letter &ref = leters[ch];
  ref = letter;
  return ref;
  }

Tempest::FontElement::LetterGeometry Tempest::FontElement::letterGeometry( char16_t ch ) const {
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

  if(fdata[key.name]==nullptr){
    RFile file(fnames[key.name].c_str());
    size_t sz = file.size();
    char * ch = new char[sz];
    fdata[key.name].reset(ch);
    if(file.readData(ch,sz)!=sz)
      fdata[key.name].reset();
    }
  Tempest::MemReader reader(fdata[key.name].get(),-1);
  FT_StreamRec stream;
  ft().mkStream(stream, reader);

  err = ft().New_Face( ft().library, stream, 0, &face );
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

const Tempest::FontElement::Letter &
  Tempest::FontElement::nullLeter(char16_t ch) const {
  Leters & leters = *lt;
  Letter letter;

  Letter &ref = leters[ch];
  ref = letter;
  //ref.surf.data.tex = 0;
  return ref;
  }

void Tempest::FontElement::fetch(const std::u16string &str,
                                 Tempest::SpritesHolder & sp  ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( str[i], sp );
  }

void Tempest::FontElement::fetch( const std::string &str,
                                  Tempest::SpritesHolder & sp ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( str[i], sp );
  }

Tempest::Size Tempest::FontElement::textSize(const std::u16string &str ) {
  return textSize( str.data(), str.data()+str.size() );
  }

Tempest::Size Tempest::FontElement::textSize(const std::string &str ) {
  return textSize( str.data(), str.data()+str.size() );
  }

Tempest::Size Tempest::FontElement::textSize(const char16_t *str) {
  const char16_t *e = str;
  while(*e) ++e;
  return textSize(str, e);
  }

Tempest::Size Tempest::FontElement::textSize(const char *str) {
  const char *e = str;
  while(*e) ++e;
  return textSize(str, e);
  }

Tempest::Size Tempest::FontElement::textSize(const char16_t *b, const char16_t *e) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( const char16_t* i=b; i<e; ++i ){
    const LetterGeometry& l = letterGeometry( *i );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw,th);
  }

Tempest::Size Tempest::FontElement::textSize(const char *b, const char *e) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( const char* i=b; i<e; ++i ){
    const LetterGeometry& l = letterGeometry( *i );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw,th);
  }

const Tempest::FontElement::Letter&
  Tempest::FontElement::letter( char16_t ch,
                         Tempest::SpritesHolder & sp ) const {
  const Tempest::FontElement::Letter& tmp = fetchLeter(ch,sp);
  return tmp;
  }

void Tempest::FontElement::update() {
  lt = ft().letterBox[key];
  if( !lt ){
    lt = new Leters();
    ft().letterBox[key] = lt;
    }
  }

Tempest::FontElement::LMap::LMap():let(0), e(0){
  std::fill( n, n+256, (LMap*)0 );
  }

Tempest::FontElement::LMap::~LMap() {
  delete[] e;
  delete[] let;

  for( int i=0; i<256; ++i )
    delete n[i];
  }


Tempest::FontElement::Letter *Tempest::FontElement::LMap::find(char16_t c) const {
  unsigned char cp[sizeof(c)];
  for( size_t i=0; i<sizeof(char16_t); ++i){
    cp[i] = c%256;
    c/=256;
    }

  const LMap *l = this;

  for( size_t i=sizeof(char16_t)-1; i>0; --i ){
    unsigned char cx = cp[i];
    if( l->n[cx]==0 )
      return 0;

    l = l->n[cx];
    }

  if( l->let==0 ){
    l->let = new Letter[256];
    l->e   = new bool[256];
    std::fill( l->e, l->e+256, false );
    }

  if( l->e[cp[0]] )
    return l->let+cp[0];

  return 0;
  }

Tempest::FontElement::Letter &Tempest::FontElement::LMap::operator [](char16_t c) {
  unsigned char cp[sizeof(c)];
  for( size_t i=0; i<sizeof(char16_t); ++i){
    cp[i] = c%256;
    c/=256;
    }

  const LMap *l = this;

  for( size_t i=sizeof(char16_t)-1; i>0; --i ){
    unsigned char cx = cp[i];
    if( l->n[cx]==0 ){
      l->n[cx] = new LMap();
      }

    l = l->n[cx];
    }

  if( l->let==0 ){
    l->let = new Letter[256];
    l->e   = new bool[256];
    std::fill( l->e, l->e+256, false );
    }

  l->e[cp[0]] = 1;
  return *(l->let+cp[0]);
  }

bool Tempest::FontElement::Key::operator < (const Tempest::FontElement::Key &other) const {
  if( size < other.size )
    return 1;
  if( size > other.size )
    return 0;
/*
  if( bold < other.bold )
    return 1;
  if( bold > other.bold )
    return 0;

  if( italic < other.italic )
    return 1;
  if( italic > other.italic )
    return 0;
  */

  return name < other.name;
  }


Tempest::Font::Font(){
  }

Tempest::Font::Font(const std::string &name, int sz) {
  ttf[0][0] = FontElement(name +   ".ttf", sz);
  ttf[0][1] = FontElement(name +  "b.ttf", sz);
  ttf[1][0] = FontElement(name +  "i.ttf", sz);
  ttf[1][1] = FontElement(name + "bi.ttf", sz);
  }

Tempest::Font::Font( const Tempest::FontElement &n,
                     const Tempest::FontElement &b,
                     const Tempest::FontElement &i,
                     const Tempest::FontElement &bi ) {
  ttf[0][0] = n;
  ttf[0][1] = b;
  ttf[1][0] = i;
  ttf[1][1] = bi;
  }

void Tempest::Font::fetch( const std::u16string &str,
                           Tempest::SpritesHolder &sp ) const {
  ttf[bold][italic].fetch(str, sp);
  }

void Tempest::Font::fetch( const std::string &str,
                           Tempest::SpritesHolder &sp ) const {
  ttf[bold][italic].fetch(str, sp);
  }

const Tempest::Font::Letter &Tempest::Font::letter( char16_t ch,
                                                    Tempest::SpritesHolder &sp) const {
  return ttf[bold][italic].letter(ch, sp);
  }

Tempest::Font::LetterGeometry Tempest::Font::letterGeometry(char16_t ch) const {
  return ttf[bold][italic].letterGeometry(ch);
  }

Tempest::Size Tempest::Font::textSize(const std::u16string &s) {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const std::string &s) {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const char16_t *b, const char16_t *e) {
  return ttf[bold][italic].textSize(b,e);
  }

Tempest::Size Tempest::Font::textSize(const char *b, const char *e) {
  return ttf[bold][italic].textSize(b,e);
  }

Tempest::Size Tempest::Font::textSize(const char16_t *s) {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const char *s) {
  return ttf[bold][italic].textSize(s);
  }

int Tempest::Font::size() const {
  return ttf[bold][italic].key.size;
  }

void Tempest::Font::setBold(bool b) {
  bold = b? 1:0;
  }

bool Tempest::Font::isBold() const {
  return bold>0;
  }

void Tempest::Font::setItalic(bool i) {
  italic = i ? 1:0;
  }

bool Tempest::Font::isItalic() const {
  return italic>0;
  }

void Tempest::Font::setSize(int s) {
  for( int i=0; i<1; ++i )
    for( int r=0; r<1; ++r ){
      ttf[i][r].key.size = s;
      ttf[i][r].update();
      }
  }
