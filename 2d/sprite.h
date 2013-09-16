#ifndef SPRITE_H
#define SPRITE_H

#include <Tempest/Utility>
#include <Tempest/SpritesHolder>
#include <vector>

namespace Tempest{

class Texture2d;


class Sprite {
  public:
    Sprite();

    int w() const;
    int h() const;

    Size size() const;

    const Tempest::Texture2d& pageRawData() const;
    Tempest::Rect pageRect() const;

    size_t handle() const;
  private:
    Tempest::Rect rect;
    SpritesHolder *holder;

    std::vector<SpritesHolder::Page> * tex;
    size_t id;

    Tempest::Texture2d * nonPool;

  friend class SpritesHolder;
  };

}

#endif // SPRITE_H
