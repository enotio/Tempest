#include "sprite.h"

using namespace Tempest;

SpritesHolder::SpriteData Sprite::noData;

Sprite::Sprite():data(&noData) {
  }

Sprite::Sprite(const Sprite &s) :data(s.data) {
  if( data!=&noData )
    data->addRef();
  }

Sprite &Sprite::operator =(const Sprite &s) {
  if( this==&s )
    return *this;

  if( data!=&noData )
    data->decRef();
  data = s.data;
  data->addRef();

  return *this;
  }

Sprite::~Sprite() {
  if( data!=&noData )
    data->decRef();
  }

bool Sprite::isDelayd() const{
  return data->page==nullptr && data!=&noData;
  }

void Sprite::flush() const {
  if( isDelayd() && data->holder )
    data->holder->loadDelayd();
  }

int Sprite::w() const {
  return data->p.width();
  }

int Sprite::h() const {
  return data->p.height();
  }

Size Sprite::size() const {
  return data->p.size();
  }

Rect Sprite::rect() const {
  return size().toRect();
  }

const Tempest::Texture2d &Sprite::pageRawData() const {
  flush();

  static const Tempest::Texture2d nullTex;
  if( data->page ){
    data->holder->flush();
    return data->page->t;
    }

  return nullTex;
  }

Size Sprite::pageRawSize() const {
  flush();

  if( data->page ){
    return data->page->p.size();
    }

  return Size();
  }

Rect Sprite::pageRect() const {
  flush();
  return data->pageRect;
  }

size_t Sprite::handle() const {
  return pageRawData().handle();
  }
