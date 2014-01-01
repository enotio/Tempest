#include "sprite.h"

using namespace Tempest;

Sprite::Sprite():holder(0), deleyd(-1), tex(0), id(-1) {

  }

Sprite::Sprite(const Sprite &s)
  :prect(s.prect), holder(s.holder), deleyd(s.deleyd), tex(s.tex), id(s.id){
  if( isDelayd() ){
    holder->delayLoad(this);
    }
  }

Sprite &Sprite::operator =(const Sprite &s) {
  if( isDelayd() ){
    holder->delayLoadRm(this);
    }

  prect  = s.prect;
  holder = s.holder;
  deleyd = s.deleyd;
  tex    = s.tex;
  id     = s.id;

  if( isDelayd() ){
    holder->delayLoad(this);
    }

  return *this;
  }

Sprite::~Sprite() {
  if( isDelayd() ){
    holder->delayLoadRm(this);
    }
  }

bool Sprite::isDelayd() const{
  return deleyd!=size_t(-1);
  }

void Sprite::flush() const {
  if( isDelayd() ){
    holder->loadDelayd();
    }
  }

int Sprite::w() const {
  flush();
  return prect.w;
  }

int Sprite::h() const {
  flush();
  return prect.h;
  }

Size Sprite::size() const {
  flush();
  return prect.size();
  }

Rect Sprite::rect() const {
  flush();
  return Rect(0,0,prect.w,prect.h);
  }

const Tempest::Texture2d &Sprite::pageRawData() const {
  flush();

  static const Tempest::Texture2d nullTex;
  if( tex )
    return (*tex)[id]->t;

  return nullTex;
  }

Rect Sprite::pageRect() const {
  flush();
  return prect;
  }

size_t Sprite::handle() const {
  return pageRawData().handle();
  }
