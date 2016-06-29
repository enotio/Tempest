#include "lineedit.h"

#include <Tempest/Application>
#include <Tempest/Android>

using namespace Tempest;

const char16_t LineEdit::passChar='*';
#ifdef __OSX__
static const KeyEvent::KeyType cmdKey = KeyEvent::K_Command;
#else
static const KeyEvent::KeyType cmdKey = KeyEvent::K_Control;
#endif

LineEdit::LineEdit(): anim(0) {
  sedit    = 0;
  eedit    = 0;
  oldSedit = 0;
  oldEedit = 0;

  resize(100,27);

  editable = 1;
  isEdited = false;

  setFocusPolicy( ClickFocus );

  onFocusChange.bind( *this, &LineEdit::storeText );

  scroll = 0;

  const UiMetrics& uiMetrics = Application::uiMetrics();
  fnt = Application::mainFont();
  fnt.setSize( int(uiMetrics.normalTextSize*uiMetrics.uiScale) );

  SizePolicy p = sizePolicy();
  p.maxSize.h = fnt.size() + fnt.size()/2;
  p.minSize.h = p.maxSize.h;
  p.typeV = FixedMax;

  setSizePolicy(p);

  timer.setRepeatCount(0);
  timer.timeout.bind( this, &LineEdit::animation );
  onFocusChange.bind( this, &LineEdit::setupTimer );
  }

LineEdit::~LineEdit() {
  onFocusChange.ubind( this, &LineEdit::setupTimer );
  onFocusChange.ubind( this, &LineEdit::storeText );
  storeText(false);
  }

void LineEdit::setEchoMode(LineEdit::EchoMode m) {
  emode = m;
  update();
  }

LineEdit::EchoMode LineEdit::echoMode() const {
  return emode;
  }

void LineEdit::setFont(const Font &f) {
  fnt = f;
  update();
  }

const Font &LineEdit::font() const {
  return fnt;
  }

void LineEdit::setText( const std::string &t ) {
  std::u16string s;
  s.assign( t.begin(), t.end() );
  setText( s );
  }

void LineEdit::setText(const std::u16string &t) {
  if( txt!=t ){
    const Validator& v = validator();
    v.assign(txt,t);

    sedit = std::max<size_t>(0, std::min(sedit, txt.size() ));
    eedit = std::max<size_t>(0, std::min(eedit, txt.size() ));

    scroll = 0;
    onTextChanged(txt);
    update();
    }
  }

const std::u16string &LineEdit::text() const {
  return txt;
  }

void LineEdit::setHint(const std::u16string &str) {
  hnt = str;
  }

void LineEdit::setHint(const std::string &str) {
  setHint( Tempest::SystemAPI::toUtf16( str ) );
  }

const std::u16string &LineEdit::hint() const {
  return hnt;
  }

size_t LineEdit::selectionBegin() {
  return sedit;
  }

size_t LineEdit::selectionEnd() {
  return eedit;
  }

void LineEdit::setSelectionBounds(size_t begin, size_t end) {
  if( begin > end )
    std::swap(begin, end);

  begin = std::min( txt.size(), begin );
  end   = std::min( txt.size(), end   );

  sedit = begin;
  eedit = end;
  }

void LineEdit::resetSelection() {
  setSelectionBounds(sedit,sedit);
  }

void LineEdit::setEditable(bool e) {
  editable = e;
  }

void LineEdit::setValidator(LineEdit::Validator *v) {
  mvalidator.reset(v);
  validator().assign(txt,txt);
  }

const LineEdit::Validator &LineEdit::validator() const {
  if(!mvalidator)
    mvalidator.reset(new Validator());
  return *mvalidator;
  }

bool LineEdit::isEditable() const {
  return editable;
  }

void LineEdit::mouseDownEvent(Tempest::MouseEvent &e) {
  if(!editable){
    e.ignore();
    return;
    }

  sp = e.pos();
  ep = e.pos();

  updateSel();
  update();

#ifdef __ANDROID__
  if( editable )
    AndroidAPI::toggleSoftInput();
#endif
  }

void LineEdit::mouseUpEvent(Tempest::MouseEvent &) {
  if( sedit > eedit )
    std::swap( sedit, eedit );
  update();
  }

void LineEdit::mouseMoveEvent(MouseEvent &) {
  Tempest::Point p = mapToRoot(Tempest::Point());
  Application::showHint(hnt, Tempest::Rect(p.x, p.y, w(), h()));
  }

void LineEdit::mouseDragEvent(MouseEvent &e) {
  ep = e.pos();
  updateSel();
  update();
  }

void LineEdit::paintEvent( Tempest::PaintEvent &pe ) {
  if(emode==NoEcho)
    return;
  Painter p(pe);

  p.setFont( fnt );

  const Margin& m = margin();
  int x = m.left, y = 0;

  size_t s = std::min( sedit, eedit );
  size_t e = std::max( sedit, eedit );

  for( size_t i=0; i<s && i<txt.size(); ++i ){
    const Font::Letter& l = emode==Normal ? p.letter(fnt, txt[i]) : p.letter(fnt,passChar);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int sx = x;

  for( size_t i=s; i<e && i<txt.size(); ++i ){
    const Font::Letter& l = emode==Normal ? p.letter(fnt, txt[i]) : p.letter(fnt,passChar);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int oldSc = scroll;
  if( editable && sx==x ){
    --sx;
    int clientW = w()-m.xMargin();
    int delta = 0;
    if( x+oldSc > clientW ){
      delta = clientW - (x+oldSc);
      }

    if( x+oldSc < m.left ){
      delta = -(x+oldSc-m.left);
      }

    scroll += delta;
    sx     += delta;
    x      += delta;
    x += 1;
    }

  p.setBlendMode(noBlend);
  if( editable && ((anim || s!=e) && hasFocus()) ){
    p.setColor( 0,0,1,1 );
    //p.setBlendMode( addBlend );
    p.drawRect( sx+oldSc, 0, x-sx, h(),
                2,0, 1,1 );
    }

  p.setColor(1,1,1,1);
  int dY = (h()-fnt.size()-m.yMargin())/2;
  Rect sc = p.scissor();
  p.setScissor(sc.intersected(Rect(m.left, 0, w()-m.xMargin(), h())));
  if(emode==Normal){
    p.drawText( scroll+m.left, m.top+dY, w()-m.xMargin()-scroll, fnt.size(),
                txt, AlignBottom );
    } else {
    const Font::Letter& l = p.letter(fnt,passChar);
    int x=scroll+m.left, y = m.top+dY;
    char16_t ch[2]={passChar,'\0'};
    for(size_t i=0;i<txt.size();++i){
      p.drawText(x,y,ch);
      x+= l.advance.x;
      y+= l.advance.y;
      }
    }
  }

void LineEdit::storeOldText(){
  oldTxt   = text();
  oldSedit = sedit;
  oldEedit = eedit;
  }

void LineEdit::undo() {
  std::swap(oldTxt,   txt  );
  std::swap(oldSedit, sedit);
  std::swap(oldEedit, eedit);

  setSelectionBounds(sedit,eedit);
  isEdited = 0;
  onEditingFinished( txt );
  }

void LineEdit::redo() {
  std::swap(oldTxt,txt);
  std::swap(oldSedit, sedit);
  std::swap(oldEedit, eedit);

  setSelectionBounds(sedit,eedit);
  isEdited = 0;
  onEditingFinished( txt );
  }

void LineEdit::keyDownEvent( KeyEvent &e ) {
  if(SystemAPI::isKeyPressed(cmdKey)){
    return;
    }

  if(!isEdited)
    storeOldText();

  if( e.u16==' ' ){
    storeOldText();
    }

  const Validator& v = validator();
  if( e.key==KeyEvent::K_NoKey && editable ){
    if( sedit < eedit && eedit-sedit==txt.size() ){
      std::u16string tmp;
      tmp.resize(1);
      tmp[0] = e.u16;
      v.assign(txt,tmp);
      sedit = txt.size();
      eedit = sedit;
      } else {
      if( sedit < eedit )
        v.erase(txt, sedit, eedit);
      v.insert(txt,sedit, eedit,e.u16);
      }

    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Return ){
    storeText(0);
    onEnter(txt);
    return;
    }

  if( e.key==KeyEvent::K_Left ){
    if( sedit>0 )
      --sedit;

    eedit = sedit;
    update();
    return;
    }

  if( e.key==KeyEvent::K_Right ){
    if( sedit<txt.size() )
      ++sedit;

    eedit = sedit;
    update();
    return;
    }

  if( e.key==KeyEvent::K_Back && editable ){    
    v.erase( txt, sedit, eedit );
    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Delete && editable ){
    if(sedit==eedit){
      ++sedit;
      ++eedit;
      }
    v.erase( txt, sedit, eedit );

    isEdited = true;
    onTextChanged( txt );
    onTextEdited(txt);
    update();
    return;
    }

  e.ignore();
  }

void LineEdit::keyUpEvent(KeyEvent &e) {
  if( SystemAPI::isKeyPressed(cmdKey) ){
    if(e.key==KeyEvent::K_Z)
      undo();
    if(e.key==KeyEvent::K_Y)
      redo();
    }
  }

void LineEdit::updateSel() {
  if(emode==NoEcho){
    sedit = txt.size();
    eedit = sedit;
    return;
    }

  Tempest::Point a = sp, b = ep;

  if( a.x > b.x )
    std::swap(a,b);

  int x = scroll+margin().left, y = 0;
  a.x = std::max(a.x,x);
  b.x = std::max(b.x,x);

  for( size_t i=0; i<txt.size(); ++i ){
    const Font::LetterGeometry& l = fnt.letterGeometry(emode==Normal ? txt[i] : passChar);

    if( Tempest::Rect( x, 0,
                       l.advance.x, h() ).contains(a,true) ){
      if( a.x < x+l.advance.x/2 )
        sedit = i; else
        sedit = i+1;
      }

    x+= l.advance.x;
    y+= l.advance.y;
    }

  if( Rect( x, y, w(), h() ).contains(a) ){
    sedit = txt.size();
    }

  if( a.x >= w() ){
    sedit = txt.size();
    eedit = txt.size();
    return;
    }

  x = scroll+margin().left;
  y = 0;

  if( b.x >= w() ){
    eedit = txt.size();
    return;
    }

  eedit = sedit;
  for( size_t i=0; i<txt.size(); ++i ){
    const Font::LetterGeometry& l = fnt.letterGeometry(emode==Normal ? txt[i] : passChar);

    if( Rect( x, 0,
              l.advance.x, h() ).contains(b,true) ){
      if( b.x < x+l.advance.x/2 )
        eedit = i; else
        eedit = i+1;
      }

    x+= l.advance.x;
    y+= l.advance.y;
    }

  if( Rect( x, y,
            w(), h() ).contains(b) ){
    eedit = txt.size();
    }
  }

void LineEdit::storeText(bool) {
  if( isEdited ){
    isEdited = 0;
    storeOldText();
    onEditingFinished( txt );
    }
  }

void LineEdit::setupTimer( bool f ) {
  if( f ){
    timer.start(700);
    } else {
    timer.stop();
    anim = 0;
    }

  update();
  }

void LineEdit::animation() {
  anim = !anim;
  update();
  }


void LineEdit::Validator::insert(std::u16string &string, size_t &ecursor, size_t &cursor, const std::u16string &data) const {
  string.insert(cursor,data);
  ++cursor;
  ++ecursor;
  }

void LineEdit::Validator::insert(std::u16string &string, size_t &cursor, size_t &ecursor, char16_t data) const {
  char16_t ch[2] = { char16_t(data), 0 };
  string.insert(cursor,ch);
  ++cursor;
  ++ecursor;
  }

void LineEdit::Validator::erase(std::u16string &string, size_t &scursor, size_t &ecursor) const {
  if( scursor < ecursor )
    string.erase( scursor, ecursor-scursor ); else
  if( scursor > 0 ){
    string.erase( ecursor-1, 1 );
    --scursor;
    }

  ecursor = scursor;
  }

void LineEdit::Validator::assign(std::u16string &string,
                                 const std::u16string &arg) const{
  string = arg;
  }

void LineEdit::IntValidator::insert(std::u16string &string,
                                    size_t &cursor, size_t &ecursor,
                                    const std::u16string &data) const {
  for(auto i:data)
    insert(string,cursor,ecursor,i);
  }

void LineEdit::IntValidator::insert(std::u16string &string,
                                    size_t &cursor, size_t &ecursor,
                                    char16_t data) const {
  if( cursor==0 ){
    if(!string.empty() && string[0]=='-')
      return;

    if(data=='-' && !(string.size()>=1 && string[0]=='0')){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }

    if(data=='0' && ((string.size()==1 && string[0]=='-') || string.size()==0)){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }

    if(('1'<=data && data<='9') || data=='-'){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }
    return;
    }

  const size_t pos = cursor-1;
  if( data=='0'
      && !(pos<string.size() && string[pos]=='-')
      && !(string.size()==1 && string[0]=='0') ){
    Validator::insert(string,cursor,ecursor,data);
    return;
    }

  if('1'<=data && data<='9'){
    if(string.size()==1 && string[0]=='0'){
      string.clear();
      cursor  = 0;
      ecursor = cursor;
      }
    Validator::insert(string,cursor,ecursor,data);
    return;
    }
  }

void LineEdit::IntValidator::erase(std::u16string &string,
                                   size_t &scursor,
                                   size_t &ecursor) const {
  Validator::erase(string,scursor,ecursor);
  if(string.size()==1 && string[0]=='-')
    string[0] = '0';

  if(string.empty())
    string.push_back('0');
  }

void LineEdit::IntValidator::assign(std::u16string &string,
                                    const std::u16string &arg) const {
  if(arg.size()==0){
    if(string.size()==0){
      string.resize(1);
      string[0] = '0';
      }
    return;
    }

  bool good=true;
  if(arg[0]=='-'){
    if(arg.size()!=1){
      if(arg[1]=='0' && arg.size()!=2)
        good=false;
      for(size_t i=1; good && i<arg.size(); ++i ){
        const char16_t c = arg[i];
        good &= ('0'<=c && c<='9');
        }
      } else {
      good = false;
      }
    } else {
    if(arg[0]=='0' && arg.size()!=1)
      good=false;
    for(size_t i=0; good && i<arg.size(); ++i ){
      const char16_t c = arg[i];
      good &= ('0'<=c && c<='9');
      }
    }

  if(good){
    string = arg;
    } else {
    if(string.size()==0){
      string.resize(1);
      string[0] = '0';
      }
    }
  }
