#include "sprite.h"

using namespace Tempest;

Sprite::Sprite():holder(0), tex(0), id(-1), nonPool(0) {

  }

int Sprite::width() const {
  return rect.w;
  }

int Sprite::height() const {
  return rect.h;
  }

Size Sprite::size() const {
  return rect.size();
  }

const Tempest::Texture2d &Sprite::pageRawData() const {
  //holder->flush();
  static const Tempest::Texture2d nullTex;
  if( tex )
    return (*tex)[id].t;

  return nullTex;
  }

Rect Sprite::pageRect() const {
  return rect;
  }

size_t Sprite::handle() const {
  return pageRawData().handle();
  }
