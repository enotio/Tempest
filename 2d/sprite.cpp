#include "sprite.h"

using namespace Tempest;

Sprite::Sprite():holder(0) {

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
  return (*tex)[id].t;
  }

Rect Sprite::pageRect() const {
  return rect;
  }
