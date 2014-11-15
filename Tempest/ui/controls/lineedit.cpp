#include "lineedit.h"

#include <Tempest/Application>
#include <Tempest/Android>

using namespace Tempest;

LineEdit::LineEdit(): anim(0), ctrlPressed(0) {
  sedit    = 0;
  eedit    = 0;
  oldSedit = 0;
  oldEedit = 0;

  resize(100,27);

  editable = 1;
  isEdited = false;

  setFocusPolicy( ClickFocus );

  onFocusChange.bind( *this, &LineEdit::storeText );

  scrool = 0;

  const UiMetrics& uiMetrics = Application::uiMetrics();
  font = Application::mainFont();
  font.setSize( int(uiMetrics.normalTextSize*uiMetrics.uiScale) );

  SizePolicy p = sizePolicy();
  p.maxSize.h = font.size() + font.size()/2;
  p.minSize.h = p.maxSize.h;
  p.typeV = FixedMax;

  setSizePolicy(p);

  timer.setRepeatCount(0);
  timer.timeout.bind( this, &LineEdit::animation );
  onFocusChange.bind( this, &LineEdit::setupTimer );
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
  if( Rect(0,0,w(),h()).contains( e.pos() ) ){
    ep = e.pos();
    updateSel();
    update();
    }
  }

void LineEdit::paintEvent( Tempest::PaintEvent &pe ) {
  Painter p(pe);

  p.setFont( font );

  int x = 0, y = 0;

  size_t s = std::min( sedit, eedit );
  size_t e = std::max( sedit, eedit );

  for( size_t i=0; i<s && i<txt.size(); ++i ){
    Font::Letter l = p.letter(font, txt[i]);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int sx = x;

  for( size_t i=s; i<e && i<txt.size(); ++i ){
    Font::Letter l = p.letter(font, txt[i]);
    x+= l.advance.x;
    y+= l.advance.y;
    }

  int oldSc = scrool;
  if( editable && sx==x ){
    --sx;
    if( x+oldSc > w() ){
      scrool += ( w() - (x+oldSc) );
      //scrool += w()/3;
      }

    if( x+oldSc < 0 ){
      scrool -= (x+oldSc);
      //scrool -= w()/3;
      }

    x += 1;
    }

  p.setBlendMode(noBlend);
  if( editable && ((anim && hasFocus()) || s!=e) ){
    p.setColor( 0,0,1,1 );
    //p.setBlendMode( addBlend );
    p.drawRect( sx+oldSc, 0, x-sx, h(),
                2,0, 1,1 );
    }

  p.setColor(1,1,1,1);
  p.drawText( scrool, 0, w()-scrool, h(), txt );
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
  if(e.key==KeyEvent::K_Control && ctrlPressed<unsigned(-1)){
    ++ctrlPressed;
    return;
    }

  if(ctrlPressed>0){
    return;
    }

  if(!isEdited)
    storeOldText();

  if( e.u16==' ' ){
    storeOldText();
    }

  const Validator& v = validator();
  if( e.key==KeyEvent::K_NoKey && editable ){
    if( sedit < eedit )
      v.erase(txt, sedit, eedit);
    v.insert(txt,sedit, eedit,e.u16);

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
  if(ctrlPressed>0){
    if(e.key==KeyEvent::K_Z)
      undo();
    if(e.key==KeyEvent::K_Y)
      redo();
    }

  if(e.key==KeyEvent::K_Control)
    ctrlPressed=0;
  }

void LineEdit::updateSel() {
  Tempest::Point a = sp, b = ep;

  if( a.x > b.x )
    std::swap(a,b);

  int x = scrool, y = 0;
  for( size_t i=0; i<txt.size(); ++i ){
    Font::LetterGeometry l = font.letterGeometry(txt[i]);

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

  x = scrool;
  y = 0;

  if( b.x >= w() ){
    eedit = txt.size();
    return;
    }

  eedit = sedit;
  for( size_t i=0; i<txt.size(); ++i ){
    Font::LetterGeometry l = font.letterGeometry(txt[i]);

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

    if(data=='0' && !(string.size()>=1 && (string[0]=='0' || string[0]=='-'))){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }

    if(('1'<=data && data<='9') || data=='-'){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }
    }

  const size_t pos = cursor-1;
  if(data=='0' && !(pos<string.size() && (string[pos]=='0' || string[pos]=='-'))){
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
