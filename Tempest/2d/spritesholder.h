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

  protected:
    virtual void onRegionFreed(Page& p,const Rect& r);

  private:
    void loadDelayd();

    Tempest::TextureHolder & holder;
    bool needToflush;

    struct Page {
      bool needToflush=true;
      std::vector< Tempest::Rect > rects;
      Tempest::Texture2d t;
      Tempest::Pixmap    p;
      };

    std::vector< std::unique_ptr<Page> > page;

    struct SpriteData {
      Tempest::Pixmap p;
      Tempest::Rect   pageRect;
      size_t          sq=0;
      Page*           page  =nullptr;
      SpritesHolder*  holder=nullptr;

      size_t          ref=1;
      void            addRef(){ref++;}
      void            decRef();
      };

    std::vector<SpriteData*> loadRq;
    std::vector<SpriteData*> spData;

    void onDelete(SpriteData* data);

    void addPage();
    bool add(SpriteData& p, Page &page );
    bool loadImpl(SpriteData& p );

    int  pageSize;

  friend class Sprite;
  friend class SurfaceRender;
  };

}

#endif // SPRITESHOLDER_H
