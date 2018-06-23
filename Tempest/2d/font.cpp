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

std::vector< std::u16string >          Tempest::FontElement::fnames;
std::vector< std::unique_ptr<char[]> > Tempest::FontElement::fdata;

Tempest::FontElement::FreeTypeLib& Tempest::FontElement::ft(){
  static FreeTypeLib lib;
  return lib;
  }

size_t Tempest::FontElement::findFontName(const std::string &n) {
  std::u16string str;
  str.assign(std::begin(n),std::end(n));

  for( size_t i=0; i<fnames.size(); ++i )
    if( fnames[i]==str )
      return i;

  fnames.push_back(std::move(str));
  fdata.resize(fnames.size());
  return fnames.size()-1;
  }

size_t Tempest::FontElement::findFontName(const std::u16string &n) {
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

Tempest::FontElement::FontElement(const std::u16string &name, int sz) {
  init(name, sz);
  }

Tempest::FontElement::FontElement(Tempest::MemReader &buffer, int sz) {
  MemFont* fnt = new MemFont();
  fnt->data    = new char[buffer.size()];
  buffer.readData(fnt->data,buffer.size());

  memData = std::shared_ptr<MemFont>(fnt);
  lt = &fnt->leters;
  key.size = sz;
  }

Tempest::FontElement::~FontElement() {
  }

Tempest::FontElement::FontElement() {
#ifdef __ANDROID__
  init("/system/fonts/DroidSans", 16);
#else
  init("./data/arial", 16);
#endif
  }

template<class Str>
void Tempest::FontElement::init(const Str &name, int sz ) {
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
    if(l->surf.w()==l->size.w && l->surf.h()==l->size.h)
      return *l;
    }

  FT_Face   face = nullptr;
  FT_Vector pen  = {0,0};
  FT_Error  err  = 0;

  char* data = nullptr;
  if(memData){
    data = memData->data;
    } else
  if(fdata[key.name]!=nullptr){
    data = fdata[key.name].get();
    } else {
    RFile file(fnames[key.name].c_str());
    size_t sz = file.size();
    data = new char[sz];
    fdata[key.name].reset(data);
    if(file.readData(data,sz)!=sz)
      fdata[key.name].reset();
    }
  Tempest::MemReader reader(data,-1);
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

    for( int r=0; r<pixmap.height(); ++r )
      for( int i=0; i<pixmap.width(); ++i ) {
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
  Leters & letters = *lt;

  if( Letter *l = letters.find(ch) ){
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
  Tempest::MemReader reader(fdata[key.name].get(),size_t(-1));
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

  Letter &ref = letters[ch];
  ref.size   =letter.size;
  ref.dpos   =letter.dpos;
  ref.advance=letter.advance;

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
                                 Tempest::SpritesHolder &sp) const {
  fetch(str.c_str(),sp);
  }

void Tempest::FontElement::fetch(const std::string &str,
                                 Tempest::SpritesHolder &sp) const {
  fetch(str.c_str(),sp);
  }

void Tempest::FontElement::fetch(const char16_t* str,
                                 Tempest::SpritesHolder & sp  ) const {
  if(!str)
    return;
  for( size_t i=0; str[i]; ++i )
    fetchLeter( str[i], sp );
  }

void Tempest::FontElement::fetch(const char *str,
                                  Tempest::SpritesHolder & sp ) const {
  if(!str)
    return;
  for( size_t i=0; str[i]; ++i )
    fetchLeter( str[i], sp );
  }

Tempest::Size Tempest::FontElement::textSize(const std::u16string &str ) const {
  return textSize( str.c_str(), str.c_str()+str.size() );
  }

Tempest::Size Tempest::FontElement::textSize(const std::string &str ) const {
  return textSize( str.c_str(), str.c_str()+str.size() );
  }

Tempest::Size Tempest::FontElement::textSize(const char16_t *str) const {
  const char16_t *e = str;
  while(*e) ++e;
  return textSize(str, e);
  }

Tempest::Size Tempest::FontElement::textSize(const char *str) const {
  const char *e = str;
  while(*e) ++e;
  return textSize(str, e);
  }

Tempest::Size Tempest::FontElement::textSize(const char16_t *b, const char16_t *e) const {
  int tx = 0, ty = 0, lx=0, ly=0, tw = 0, th = 0;
  if(b!=e){
    const LetterGeometry& l = letterGeometry( *b );
    lx = l.dpos.x;
    ly = l.dpos.y;
    }

  for( const char16_t* i=b; i<e; ++i ){
    const LetterGeometry& l = letterGeometry( *i );

    lx = std::min( tx+l.dpos.x, lx);
    ly = std::min( ty+l.dpos.y, ly);
    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw-lx,th-ly);
  }

Tempest::Size Tempest::FontElement::textSize(const char *b, const char *e) const {
  int tx = 0, ty = 0, lx = 0, ly = 0, tw = 0, th = 0;
  if(b!=e){
    const LetterGeometry& l = letterGeometry( *b );
    lx = l.dpos.x;
    ly = l.dpos.y;
    }

  for( const char* i=b; i<e; ++i ){
    const LetterGeometry& l = letterGeometry( *i );

    lx = std::min( tx+l.dpos.x, lx);
    ly = std::min( ty+l.dpos.y, ly);
    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw-lx,th-ly);
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
  ttf[0][1] = FontElement(name +  "i.ttf", sz);
  ttf[1][0] = FontElement(name +  "b.ttf", sz);
  ttf[1][1] = FontElement(name + "bi.ttf", sz);
  }

Tempest::Font::Font(const std::u16string &name, int sz) {
  std::u16string n0 = name;
  n0.append(".ttf",".ttf"+4);
  ttf[0][0] = FontElement(n0, sz);
  n0 = name;
  n0.append("i.ttf","i.ttf"+5);
  ttf[0][1] = FontElement(n0, sz);
  n0 = name;
  n0.append("b.ttf","b.ttf"+5);
  ttf[1][0] = FontElement(n0, sz);
  n0 = name;
  n0.append("i.ttf","i.ttf"+5);
  ttf[1][1] = FontElement(n0, sz);
  }

Tempest::Font::Font( const Tempest::FontElement &n,
                     const Tempest::FontElement &b,
                     const Tempest::FontElement &i,
                     const Tempest::FontElement &bi ) {
  ttf[0][0] = n;
  ttf[0][1] = i;
  ttf[1][0] = b;
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

void Tempest::Font::fetch(const char16_t *str, Tempest::SpritesHolder &sp) const {
  ttf[bold][italic].fetch(str, sp);
  }

void Tempest::Font::fetch(const char *str, Tempest::SpritesHolder &sp) const {
  ttf[bold][italic].fetch(str, sp);
  }

const Tempest::Font::Letter &Tempest::Font::letter( char16_t ch,
                                                    Tempest::SpritesHolder &sp) const {
  return ttf[bold][italic].letter(ch, sp);
  }

Tempest::Font::LetterGeometry Tempest::Font::letterGeometry(char16_t ch) const {
  return ttf[bold][italic].letterGeometry(ch);
  }

Tempest::Size Tempest::Font::textSize(const std::u16string &s) const {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const std::string &s) const {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const char16_t *b, const char16_t *e) const {
  return ttf[bold][italic].textSize(b,e);
  }

Tempest::Size Tempest::Font::textSize(const char *b, const char *e) const {
  return ttf[bold][italic].textSize(b,e);
  }

Tempest::Size Tempest::Font::textSize(const char16_t *s) const {
  return ttf[bold][italic].textSize(s);
  }

Tempest::Size Tempest::Font::textSize(const char *s) const {
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
  for( int i=0; i<2; ++i )
    for( int r=0; r<2; ++r ){
      ttf[i][r].key.size = s;
      ttf[i][r].update();
      }
  }

Tempest::FontElement::MemFont::~MemFont() {
  delete[] data;
  }
