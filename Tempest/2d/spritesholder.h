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
    virtual ~SpritesHolder();

    virtual Sprite load( const char* f );
    virtual Sprite load( const std::string& f );
    virtual Sprite load( const Tempest::Pixmap & p );

    virtual void flush();
  private:
    void   loadDelayd();
    Size   spriteSizeD(size_t deleyd );
    void   delayLoad  ( Sprite* s );
    void   delayLoadRm( Sprite* s );

    Sprite loadImpl( const Tempest::Pixmap & p );

    Tempest::TextureHolder & holder;
    bool needToflush;

    struct Page{
      std::vector< Tempest::Rect > rects;
      Tempest::Texture2d t;
      Tempest::Pixmap    p;
      };

    std::vector< std::unique_ptr<Page> > page;

    struct LoadRq{
      Tempest::Pixmap p;
      size_t          sq;
      Sprite*         sprite;
      std::vector<Sprite*> spr;
      };

    std::vector<LoadRq> loadRq;

    void   addPage();
    Sprite add( const Tempest::Pixmap & p, Page &page );

    int pageSize;

  friend class Sprite;
  friend class SurfaceRender;
  };

}

#endif // SPRITESHOLDER_H
