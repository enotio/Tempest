#include "textmodel.h"

using namespace Tempest;

TextModel::TextModel() {
  }

TextModel::~TextModel() {
  }

const std::u16string &TextModel::text() const {
  return data;
  }

void TextModel::cutUndo(size_t max) {
  if(undoListSize<=max)
    return;
  Cmd* cmd=undoList.get();
  for(size_t i=0;i<max;++i)
    cmd=cmd->next.get();
  if( cmd )
    cmd->next.reset();
  }

void TextModel::exec(TextModel::Cmd *c) {
  cutUndo(maxUndo);

  c->next.reset( undoList.release() );
  undoList.reset(c);
  undoListSize++;
  c->redo(*this);
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

bool TextModel::undo() {
  if(undoListSize==0)
    return false;
  Cmd* c =undoList.release();
  undoList.reset(c->next.release());
  undoListSize--;
  c->undo(*this);
  delete c;
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
