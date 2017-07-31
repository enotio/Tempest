#include "lineedit.h"

#include <Tempest/Application>
#include <Tempest/Android>

using namespace Tempest;

#ifdef __OSX__
static const KeyEvent::KeyType cmdKey = KeyEvent::K_Command;
#else
static const KeyEvent::KeyType cmdKey = KeyEvent::K_Control;
#endif

LineEdit::LineEdit() {
  resize(100,27);
  setFocusPolicy( WheelFocus );

  const UiMetrics& uiMetrics = Application::uiMetrics();

  Font fnt = Application::mainFont();
  fnt.setSize( int(uiMetrics.normalTextSize*uiMetrics.uiScale) );
  txt.setDefaultFont(fnt);

  SizePolicy p = sizePolicy();
  p.maxSize.h  = fnt.size() + fnt.size()/2;
  p.minSize.h  = p.maxSize.h;
  p.typeV      = FixedMax;

  setSizePolicy(p);
  }

LineEdit::~LineEdit() {
  storeText();
  }

void LineEdit::setEchoMode(LineEdit::EchoMode m) {
  auto st=state();
  st.echo = m;
  setWidgetState(st);
  }

LineEdit::EchoMode LineEdit::echoMode() const {
  return state().echo;
  }

void LineEdit::setTabChangesFocus(bool ch) {
  tabChFocus=ch;
  }

bool LineEdit::tabChangesFocus() const {
  return tabChFocus;
  }

void LineEdit::setFont(const Font &f) {
  txt.setDefaultFont(f);
  update();
  }

const Font &LineEdit::font() const {
  return txt.defaultFont();
  }

void LineEdit::setText( const std::string &t ) {
  std::u16string s;
  s.assign( t.begin(), t.end() );
  setText( s );
  }

void LineEdit::setText(const std::u16string &t) {
  if( txt.text()!=t ){
    const Validator& v = validator();
    v.assign(txt,t);
    txt.clearSteps();

    scroll = 0;
    onTextChanged(txt.text());
    update();
    }
  }

void LineEdit::setTextColor(const Color& c) {
  tColor = c;
  update();
  }

const Color& LineEdit::textColor() const {
  return tColor;
  }

const std::u16string &LineEdit::text() const {
  return txt.text();
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

size_t LineEdit::selectionBegin() const {
  return txt.selectionBegin();
  }

size_t LineEdit::selectionEnd()   const {
  return txt.selectionEnd();
  }

void LineEdit::setSelectionBounds(size_t begin, size_t end) {
  txt.setSelectionBounds(begin,end);
  }

void LineEdit::resetSelection() {
  setSelectionBounds(selectionBegin(),selectionBegin());
  }

size_t LineEdit::cursorForPosition(const Point &pos) const {
  const Margin& m = margin();
  return txt.cursorForPosition(pos-Point(m.left,m.top),echoMode());
  }

void LineEdit::setEditable(bool e) {
  auto st = state();
  st.editable=e;
  setWidgetState(st);
  }

void LineEdit::setValidator(LineEdit::Validator *v) {
  mvalidator.reset(v);
  validator().assign(txt,txt.text());
  txt.clearSteps();
  }

const LineEdit::Validator &LineEdit::validator() const {
  if(!mvalidator)
    mvalidator.reset(new Validator());
  return *mvalidator;
  }

bool LineEdit::isEditable() const {
  return state().editable;
  }

void LineEdit::mouseDownEvent(Tempest::MouseEvent &e) {
  if(!isEditable()){
    e.ignore();
    return;
    }
  if(!isEnabled())
    return;

  pressPos = cursorForPosition(e.pos());
  txt.setSelectionBounds(pressPos,pressPos);

  updateSel();
  update();

#ifdef __ANDROID__
  if( isEditable() )
    AndroidAPI::toggleSoftInput();
#endif
  }

void LineEdit::mouseDragEvent(MouseEvent &e) {
  if(!isEnabled())
    return;
  const size_t pos = cursorForPosition(e.pos());

  if( pressPos<=pos )
    txt.setSelectionBounds(pressPos,pos);else
    txt.setSelectionBounds(pos,pressPos);

  updateSel();
  update();
  }

void LineEdit::mouseUpEvent(Tempest::MouseEvent &) {
  updateSel();
  update();
  }

void LineEdit::mouseMoveEvent(MouseEvent &) {
  Tempest::Point p = mapToRoot(Tempest::Point());
  Application::showHint(hnt, Tempest::Rect(p.x, p.y, w(), h()));
  }

void LineEdit::paintEvent( PaintEvent &e ) {
  Painter p(e);

  style().draw(p,this,Style::E_Background,      state(),Rect(0,0,w(),h()),Style::Extra(*this));
  style().draw(p,txt, Style::TE_LineEditContent,state(),Rect(0,0,w(),h()),Style::Extra(*this));
  paintNested(e);
  }

void LineEdit::resizeEvent(SizeEvent&) {
  Size sz = size();
  sz.w -= margin().xMargin();
  sz.h -= margin().yMargin();
  txt.setViewport(sz);
  }


void LineEdit::undo() {
  txt.undo();
  isEdited = false;

  onTextChanged    (txt.text());
  onEditingFinished(txt.text());

  update();
  }

void LineEdit::redo() {
  txt.redo();
  isEdited = false;

  onTextChanged    (txt.text());
  onEditingFinished(txt.text());

  update();
  }

void LineEdit::drawCursor(Painter &p,int x1,int x2, bool animState) {
  if( isEditable() && ((animState || selectionBegin()!=selectionEnd()) && hasFocus()) ){
    p.setBlendMode(noBlend);
    p.unsetTexture();
    p.setColor( 0,0,1,1 );
    p.drawRect( x1, 0, x2, h() );
    }
  }

void LineEdit::keyDownEvent( KeyEvent &e ) {
  if(!isEnabled())
    return;

  if(e.key==Event::K_Tab && (tabChangesFocus() || SystemAPI::isKeyPressed(cmdKey)) ) {
    focusTraverse( !SystemAPI::isKeyPressed(Event::K_Shift) );
    return;
    }

  if(SystemAPI::isKeyPressed(cmdKey)){
    return;
    }

  size_t sedit = txt.selectionBegin();
  size_t eedit = txt.selectionEnd();

  const Validator& v = validator();
  if( e.key==KeyEvent::K_NoKey && isEditable() ){
    if( eedit-sedit==txt.size() ){
      std::u16string tmp;
      tmp.resize(1);
      tmp[0] = e.u16;
      v.assign(txt,tmp);
      txt.setSelectionBounds(txt.size());
      } else {
      if( sedit < eedit )
        v.erase(txt,sedit,eedit);
      v.insert(txt,sedit,eedit,e.u16);
      txt.setSelectionBounds(sedit);
      }

    isEdited = true;
    onTextChanged( txt.text() );
    onTextEdited ( txt.text() );
    update();
    return;
    }

  if( e.key==KeyEvent::K_Return ){
    storeText();
    onEnter(txt.text());
    return;
    }

  if( e.key==KeyEvent::K_Left ){
    if( sedit>0 )
      txt.setSelectionBounds(sedit-1);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Right ){
    if( sedit<txt.size() )
      txt.setSelectionBounds(sedit+1);
    update();
    return;
    }

  if( e.key==KeyEvent::K_Back && isEditable() ){
    v.erase( txt, sedit, eedit );
    txt.setSelectionBounds(sedit,eedit);

    isEdited = true;
    onTextChanged( txt.text() );
    onTextEdited ( txt.text() );
    update();
    return;
    }

  if( e.key==KeyEvent::K_Delete && isEditable() ){
    if(sedit==eedit){
      ++sedit;
      ++eedit;
      }
    v.erase( txt, sedit, eedit );
    txt.setSelectionBounds(sedit,eedit);

    isEdited = true;
    onTextChanged( txt.text() );
    onTextEdited ( txt.text() );
    update();
    return;
    }

  e.ignore();
  }

void LineEdit::keyUpEvent(KeyEvent &e) {
  if( SystemAPI::isKeyPressed(cmdKey) && isEnabled() ){
    if(e.key==KeyEvent::K_Z)
      undo();
    if(e.key==KeyEvent::K_Y)
      redo();
    }
  }

void LineEdit::focusEvent(FocusEvent &e) {
  storeText();
  if( e.reason==Event::TabReason && e.in ){
    setSelectionBounds(0,text().size());
    }
  }

void LineEdit::updateSel() {
  if(echoMode()==NoEcho)
    txt.setSelectionBounds(txt.size(),txt.size());
  }

void LineEdit::storeText() {
  if( isEdited ){
    isEdited = false;
    onEditingFinished( txt.text() );
    }
  }

void LineEdit::setWidgetState(const WidgetState &s) {
  setFocusPolicy( s.editable ? WheelFocus : NoFocus );
  Widget::setWidgetState(s);
  update();
  }


void LineEdit::Validator::insert(TextModel &string, size_t &ecursor, size_t &cursor, const std::u16string &data) const {
  string.insert(cursor,data);
  ++cursor;
  ++ecursor;
  }

void LineEdit::Validator::insert(TextModel &string, size_t &cursor, size_t &ecursor, char16_t data) const {
  char16_t ch[2] = { char16_t(data), 0 };
  string.insert(cursor,ch);
  ++cursor;
  ++ecursor;
  }

void LineEdit::Validator::erase(TextModel &string, size_t &scursor, size_t &ecursor) const {
  if( scursor < ecursor )
    string.erase( scursor, ecursor-scursor ); else
  if( scursor > 0 ){
    string.erase( ecursor-1, 1 );
    --scursor;
    }

  ecursor = scursor;
  }

void LineEdit::Validator::assign(TextModel &string, const std::u16string &arg) const{
  string.assign(arg);
  }

void LineEdit::IntValidator::insert(TextModel &string,size_t &cursor, size_t &ecursor,const std::u16string &data) const {
  for(auto i:data)
    insert(string,cursor,ecursor,i);
  }

void LineEdit::IntValidator::insert(TextModel &string,size_t &cursor, size_t &ecursor,char16_t data) const {
  const std::u16string& chars=string.text();
  if( cursor==0 ){
    if(!chars.empty() && chars[0]=='-')
      return;

    if(data=='-' && !(chars.size()>=1 && chars[0]=='0')){
      Validator::insert(string,cursor,ecursor,data);
      return;
      }

    if(data=='0' && ((chars.size()==1 && chars[0]=='-') || chars.size()==0)){
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
      && !(pos<chars.size() && chars[pos]=='-')
      && !(chars.size()==1  && chars[0  ]=='0') ){
    Validator::insert(string,cursor,ecursor,data);
    return;
    }

  if('1'<=data && data<='9'){
    if(chars.size()==1 && chars[0]=='0'){
      string.clear();
      cursor  = 0;
      ecursor = 0;
      }
    Validator::insert(string,cursor,ecursor,data);
    return;
    }
  }

void LineEdit::IntValidator::erase(TextModel &string,
                                   size_t    &scursor,
                                   size_t    &ecursor) const {
  Validator::erase(string,scursor,ecursor);
  if(string.size()==1 && string.text()[0]=='-')
    string.assign(u"0");

  if(string.empty())
    string.insert(0,u'0');
  }

void LineEdit::IntValidator::assign(TextModel &string,
                                    const std::u16string &arg) const {
  if(arg.size()==0){
    if(string.size()==0){
      string.assign(u"0");
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
    string.assign(arg);
    } else {
    if(string.size()==0){
      string.assign(u"0");
      }
    }
  }
