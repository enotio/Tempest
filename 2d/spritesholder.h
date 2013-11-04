#ifndef SPRITESHOLDER_H
#define SPRITESHOLDER_H

#include <Tempest/TextureHolder>
#include <Tempest/Pixmap>
#include <Tempest/Texture2d>
#include <Tempest/Utility>
#include <vector>

namespace Tempest{

class Sprite;

class SpritesHolder {
    struct Page;
  public:
    SpritesHolder( Tempest::TextureHolder & h );

    virtual Sprite load( const char* f );
    virtual Sprite load( const std::string& f );
    virtual Sprite load( const Tempest::Pixmap & p );

    void flush();
  private:
    Tempest::TextureHolder & holder;
    bool needToflush;

    struct Page{
      std::vector< Tempest::Rect > rects;
      Tempest::Texture2d t;
      Tempest::Pixmap    p;
      };

    std::vector<Page> page;

    void   addPage();
    Sprite add( const Tempest::Pixmap & p, Page &page );

    int pageSize;

  friend class Sprite;
  };

}

#endif // SPRITESHOLDER_H