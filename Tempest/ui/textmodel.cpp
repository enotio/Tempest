#include "textmodel.h"

#include <Tempest/Application>
#include <locale>

using namespace Tempest;

const char16_t AbstractTextModel::passChar='*';

void AbstractTextModel::UndoQueue::push(AbstractTextModel::Cmd *c) {
  c->next.reset( list.release() );
  list.reset(c);
  size++;
  }

AbstractTextModel::Cmd *AbstractTextModel::UndoQueue::pop() {
  if(size==0)
    return nullptr;

  Cmd* c =list.release();
  list.reset(c->next.release());
  size--;
  return c;
  }

void AbstractTextModel::UndoQueue::cut(size_t max) {
  if(size<=max)
    return;

  if(max==0) {
    clear();
    return;
    }

  size=max;
  Cmd* cmd=list.get();
  for(size_t i=1;i<max;++i)
    cmd=cmd->next.get();
  if( cmd )
    cmd->next.reset();
  }

void AbstractTextModel::UndoQueue::clear() {
  list.reset();
  size=0;
  }


AbstractTextModel::AbstractTextModel() : fnt(Application::mainFont()) {
  }

AbstractTextModel::~AbstractTextModel() {
  }

size_t AbstractTextModel::size() const {
  return text().size();
  }


void AbstractTextModel::exec(AbstractTextModel::InsChar *c) {
  if(exec(reinterpret_cast<Cmd*>(c)))
    insChain=c;
  }

void AbstractTextModel::exec(AbstractTextModel::RmChar *c) {
  if(exec(reinterpret_cast<Cmd*>(c)))
    rmChain =c;
  }

bool AbstractTextModel::exec(AbstractTextModel::Cmd *c) {
  clearChains();
  redoList.clear();

  if(maxUndo>0) {
    undoList.cut(maxUndo-1);
    undoList.push(c);
    }
  c->redo(*this);
  if(maxUndo==0) {
    delete c;
    return false;
    }
  return true;
  }

void AbstractTextModel::clearChains() {
  insChain=nullptr;
  rmChain =nullptr;
  }

void AbstractTextModel::append(const char16_t *str) {
  insert(size(),str);
  }

void AbstractTextModel::insert(size_t pos,const char16_t ch) {
  if(insChain!=nullptr) {
    if(insChain->patch(*this,pos,ch))
      return;
    insChain=nullptr;
    }
  exec(new InsChar(pos,ch));
  }

void AbstractTextModel::insert(size_t pos,const char16_t *str) {
  if(str[0]=='\0')
    return;
  if(str[1]=='\0')
    return insert(pos,str[0]);
  exec(new InsChar(pos,str));
  }

void AbstractTextModel::insert(size_t pos, const std::u16string &str) {
  if(str.size()==0)
    return;
  if(str.length()==1)
    return insert(pos,str[0]);
  exec(new InsChar(pos,str));
  }

void AbstractTextModel::erase(size_t pos, size_t sz) {
  if(sz==0)
    return;

  if(rmChain!=nullptr) {
    if(rmChain->patch(*this,pos,sz))
      return;
    rmChain=nullptr;
    }
  exec(new RmChar(pos,sz));
  }

void AbstractTextModel::clear() {
  erase(0,size());
  }

void AbstractTextModel::replace(size_t pos, size_t sz, const char16_t *str) {
  if(sz==0)
    return insert(pos,str);
  exec(new Replace(pos,sz,str));
  }

void AbstractTextModel::replace(size_t pos, size_t sz, const std::u16string &str) {
  if(sz==0)
    return insert(pos,str);
  exec(new Replace(pos,sz,str));
  }

void AbstractTextModel::assign(const char16_t*      str) {
  replace(0,size(),str);
  }

void AbstractTextModel::assign(const std::u16string &str) {
  replace(0,size(),str);
  }

void AbstractTextModel::setMaxUndoSteps(size_t sz) {
  maxUndo=sz;
  undoList.cut(maxUndo);
  redoList.cut(maxUndo);
  if(maxUndo==0)
    clearChains();
  }

bool AbstractTextModel::undo() {
  Cmd* c=undoList.pop();
  if(!c)
    return false;

  clearChains();
  c->undo(*this);
  redoList.cut(maxUndo-1);
  redoList.push(c);
  return true;
  }

bool AbstractTextModel::redo() {
  Cmd* c=redoList.pop();
  if(!c)
    return false;

  clearChains();
  c->redo(*this);
  undoList.cut(maxUndo-1);
  undoList.push(c);
  return true;
  }

size_t AbstractTextModel::availableUndoSteps() const {
  return undoList.size;
  }

size_t AbstractTextModel::availableRedoSteps() const {
  return redoList.size;
  }

void AbstractTextModel::clearSteps() {
  undoList.clear();
  redoList.clear();

  insChain=nullptr;
  rmChain =nullptr;
  }

const Font &AbstractTextModel::defaultFont() const {
  return fnt;
  }

void AbstractTextModel::setDefaultFont(const Font &font) {
  fnt = font;
  }

void AbstractTextModel::setViewport(const Size &s) {
  sz = s;
  }

const Size &AbstractTextModel::viewport() const {
  return sz;
  }

size_t AbstractTextModel::cursorForPosition(const Point &pos, const WidgetState::EchoMode echoMode) const {
  const std::u16string& txt=text();
  int    x=0,y=0;

  if(pos.x<=0)
    return 0;

  if(pos.x>=sz.w)
    return txt.size();

  for( size_t i=0; i<txt.size(); ++i ) {
    const Font::LetterGeometry& l = fnt.letterGeometry(echoMode==WidgetState::Normal ? txt[i] : passChar);

    if( x<=pos.x && pos.x<=x+l.advance.x ){ //Tempest::Rect(x,0,l.advance.x,fnt.size()).contains(pos,true) ){
      if( pos.x < x+l.advance.x/2 )
        return i; else
        return i+1;
      }

    x+= l.advance.x;
    y+= l.advance.y;
    }

  return txt.size();
  }

AbstractTextModel::CharCategory AbstractTextModel::charCategory(const char16_t c) {
  if(c==' ' || c=='\t')
    return CC_Space;
  if(c=='\r' || c=='\n')
    return CC_LineBreak;
  return CC_Normal;
  }


AbstractTextModel::InsChar::InsChar(size_t pos, const char16_t c) : pos(pos){
  val.push_back(c);
  }

AbstractTextModel::InsChar::InsChar(size_t pos, const char16_t *c) : val(c), pos(pos) {
  }

AbstractTextModel::InsChar::InsChar(size_t pos, const std::u16string &c) : val(c), pos(pos) {
  }

void AbstractTextModel::InsChar::redo(AbstractTextModel &m) {
  m.rawInsert(pos,val);
  }

void AbstractTextModel::InsChar::undo(AbstractTextModel &m) {
  m.rawErase(pos,val.size(),nullptr);
  }

bool AbstractTextModel::InsChar::patch(AbstractTextModel &m, size_t npos, const char16_t c) {
  if(charCategory(c)!=charCategory(val.back()))
    return false;

  if(pos+val.size()==npos){
    m.rawInsert(pos+val.size(),c);
    val.push_back(c);
    return true;
    }
  return false;
  }

AbstractTextModel::RmChar::RmChar(size_t pos, size_t sz) : pos(pos) {
  val.resize(sz);
  }

void AbstractTextModel::RmChar::redo(AbstractTextModel &m) {
  m.rawErase(pos,val.size(),&val[0]);
  }

void AbstractTextModel::RmChar::undo(AbstractTextModel &m) {
  m.rawInsert(pos,val);
  }

bool AbstractTextModel::RmChar::patch(AbstractTextModel &m,size_t npos, size_t nsz) {
  if(nsz!=1)
    return false;

  if(pos==npos) {
    size_t oldSize=val.size();
    val.resize(oldSize+nsz,'\0');
    m.rawErase(pos,nsz,&val[oldSize]);
    return true;
    }
  if(npos+nsz==pos){
    val.insert(0,nsz,'\0');
    m.rawErase(npos,nsz,&val[0]);
    pos-=nsz;
    return true;
    }
  return false;
  }

AbstractTextModel::Replace::Replace(size_t pos,size_t sz,const char16_t *str) : val(str), old(sz,'\0'), pos(pos) {
  }

AbstractTextModel::Replace::Replace(size_t pos,size_t sz,const std::u16string &str) : val(str), old(sz,'\0'), pos(pos) {
  }

void AbstractTextModel::Replace::redo(AbstractTextModel &m) {
  m.rawErase (pos,old.size(),&old[0]);
  m.rawInsert(pos,val);
  }

void AbstractTextModel::Replace::undo(AbstractTextModel &m) {
  m.rawErase (pos,val.size(),nullptr);
  m.rawInsert(pos,old);
  }


const std::u16string &TextModel::text() const {
  return data;
  }

void TextModel::rawInsert(size_t pos, const char16_t str) {
  data.insert(pos,1,str);
  }

void TextModel::rawInsert(size_t pos, const std::u16string &str) {
  data.insert(pos,str);
  }

void TextModel::rawErase(size_t pos, size_t count, char16_t *outbuf) {
  if( outbuf ) {
    for(size_t i=0;i<count;++i)
      outbuf[i]=data[pos+i];
    }
  data.erase(pos,count);
  }

