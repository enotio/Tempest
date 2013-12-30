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
    Sprite( const Sprite& );
    Sprite& operator = ( const Sprite& );
    virtual ~Sprite();

    int w() const;
    int h() const;

    Size size() const;

    const Tempest::Texture2d& pageRawData() const;
    Tempest::Rect pageRect() const;

    size_t handle() const;

    bool   isDelayd() const;
    void   flush() const;
  private:
    Tempest::Rect    rect;
    SpritesHolder   *holder;
    size_t           deleyd;

    std::vector< std::unique_ptr<SpritesHolder::Page> > * tex;
    size_t id;

  friend class SurfaceRender;
  friend class SpritesHolder;
  };

}

#endif // SPRITE_H
