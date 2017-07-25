#include "textmodel.h"

using namespace Tempest;

TextModel::TextModel() {
  }

TextModel::~TextModel() {
  }

const std::u16string &TextModel::text() const {
  return data;
  }

void TextModel::exec(TextModel::Cmd *c) {
  if(maxUndo>0) {
    undoList.cut(maxUndo-1);
    undoList.push(c);
    }
  c->redo(*this);
  if(maxUndo==0)
    delete c;
  }

void TextModel::append(const char16_t *ch) {
  insert(data.size(),ch);
  }

void TextModel::insert(size_t pos,const char16_t ch) {
  exec(new InsChar(pos,ch));
  }

void TextModel::insert(size_t pos,const char16_t *ch) {
  exec(new InsChar(pos,ch));
  }

void TextModel::erase(size_t pos, size_t sz) {
  exec(new RmChar(pos,sz));
  }

void TextModel::setMaxUndoSteps(size_t sz) {
  maxUndo=sz;
  undoList.cut(maxUndo);
  redoList.cut(maxUndo);
  }

bool TextModel::undo() {
  Cmd* c=undoList.pop();
  if(!c)
    return false;

  c->undo(*this);
  redoList.cut(maxUndo-1);
  redoList.push(c);
  return true;
  }


TextModel::InsChar::InsChar(size_t pos, const char16_t c) : pos(pos){
  val.push_back(c);
  }

TextModel::InsChar::InsChar(size_t pos, const char16_t *c) : val(c), pos(pos) {
  }

void TextModel::InsChar::redo(TextModel &m) {
  m.data.insert(pos,val);
  }

void TextModel::InsChar::undo(TextModel &m) {
  m.data.erase(pos,val.size());
  }

TextModel::RmChar::RmChar(size_t pos, size_t sz) : pos(pos) {
  val.resize(sz);
  }

void TextModel::RmChar::redo(TextModel &m) {
  size_t sz=val.size();
  for(size_t i=0;i<sz;++i)
    val[i]=m.data[pos+sz];
  m.data.erase(pos,sz);
  }

void TextModel::RmChar::undo(TextModel &m) {
  m.data.insert(pos,val);
  }

void TextModel::UndoQueue::push(TextModel::Cmd *c) {
  c->next.reset( list.release() );
  list.reset(c);
  size++;
  }

TextModel::Cmd *TextModel::UndoQueue::pop() {
  if(size==0)
    return nullptr;

  Cmd* c =list.release();
  list.reset(c->next.release());
  size--;
  return c;
  }

void TextModel::UndoQueue::cut(size_t max) {
  if(size<=max)
    return;

  if(max==0) {
    clear();
    return;
    }

  Cmd* cmd=list.get();
  for(size_t i=1;i<max;++i)
    cmd=cmd->next.get();
  if( cmd )
    cmd->next.reset();
  }

void TextModel::UndoQueue::clear() {
  list.reset();
  size=0;
  }
