#include "spritesholder.h"

#include <cassert>
#include <string>
#include <algorithm>

#include <Tempest/Sprite>
#include <Tempest/Device>


using namespace Tempest;

SpritesHolder::SpritesHolder(Tempest::TextureHolder &h):holder(h) {
  needToflush = false;
  pageSize    = std::min( h.device().caps().maxTextureSize, 2048 );
  page.reserve(2);
  addPage();
  }

SpritesHolder::~SpritesHolder() {
  for(LoadRq& l : loadRq){
    if( l.sprite )
      l.sprite->reset();

    for( size_t i=0; i<l.spr.size(); ++i )
      l.spr[i]->reset();
    }
  }

Sprite SpritesHolder::load(const char *f) {
  return load( Pixmap(f) );
  }

Sprite SpritesHolder::load(const std::string &f) {
  return load( Pixmap(f) );
  }

Sprite SpritesHolder::load(const Tempest::Pixmap &p) {
  Sprite tmp;
  tmp.delayd = loadRq.size();
  tmp.holder = this;

  LoadRq l;
  l.p  = p;
  l.sprite = 0;
  l.sq = p.width()*p.height();
  loadRq.push_back(l);

  delayLoad(&tmp);
  return tmp;
  }

void SpritesHolder::delayLoad( Sprite *s ) {
  LoadRq &l = loadRq[s->delayd];
  if( l.sprite==0 )
    l.sprite = s; else
    l.spr.push_back(s);
  }

void SpritesHolder::delayLoadRm( Sprite *s ) {
  LoadRq &l = loadRq[s->delayd];
  if( l.sprite==s )
    l.sprite = 0;

  for( size_t i=0; i<l.spr.size(); ++i ){
    if( l.spr[i]==s ){
      l.spr[i] = l.spr.back();
      l.spr.pop_back();
      }
    }
  }

Sprite SpritesHolder::loadImpl(const Tempest::Pixmap &p) {
  if( p.width()==0 || p.height()==0 ){
    Sprite n;
    n.tex = 0;
    n.id  = 0;
    return n;
    }

  if( p.width()  > pageSize ||
      p.height() > pageSize ){
    page.emplace_back( new Page() );
    Page &pg = *page.back();
    pg.p = p;

    Sprite n;
    n.tex    = &page;
    n.holder = this;
    n.id     = page.size()-1;
    return n;
    }

  for( size_t i=0; i<page.size(); ++i ){
    Sprite r = add( p, *page[i] );
    if( r.tex ){
      r.id  = i;
      r.tex = &page;
      r.holder = this;

      return r;
      }

    if( i+1==page.size() )
      addPage();
    }

  return Sprite();
  }

void SpritesHolder::flush() {
  if( needToflush ){
    for( size_t i=0; i<page.size(); ++i ){
      page[i]->t = holder.create( page[i]->p, false, false );

      Tempest::Texture2d::Sampler s;
      s.mipFilter = Tempest::Texture2d::FilterType::Nearest;
      s.magFilter = s.mipFilter;
      s.minFilter = s.mipFilter;
      s.anisotropic = false;

      page[i]->t.setSampler(s);

      //std::string str = "./debug*.png";
      //str[7] = i+'0';
      //page[i]->p.save( str );
      }

    //page[0].p.save("./debug.png");
    }
  needToflush = false;
  }

void SpritesHolder::loadDelayd() {
  struct Grater{
    static bool cmp( const LoadRq& a, const LoadRq& b ) {
      return a.sq > b.sq;
      }
    };

  std::sort( loadRq.begin(), loadRq.end(), Grater::cmp );

  for( size_t i=0; i<loadRq.size(); ++i ){
    LoadRq& rq = loadRq[i];
    Sprite sx = loadImpl(rq.p);

    if( rq.sprite ){
      rq.sprite->delayd = -1;
      *rq.sprite = sx;
      }

    for( Sprite* s:rq.spr ){
      s->delayd = -1;
      *s = sx;
      }
    }

  loadRq.clear();
  }

Size SpritesHolder::spriteSizeD(size_t delayd) {
  return loadRq[delayd].p.size();
  }

void SpritesHolder::addPage() {
  page.emplace_back( new Page() );
  Page& p = *page.back();

  p.p = Tempest::Pixmap(pageSize, pageSize, true);
  p.rects.push_back( Tempest::Rect(0,0, p.p.width(), p.p.height() ) );
  }

Sprite SpritesHolder::add(const Tempest::Pixmap &px, Page & page ) {
  needToflush = true;

  page.t = Tempest::Texture2d();

  size_t id = 0, sq = 0;
  for( size_t i=0; i<page.rects.size(); ++i ){
    const Tempest::Rect r = page.rects[i];

    if( r.w>=px.width() && r.h>=px.height() ){
      size_t sq2 = r.w*r.h;
      if( sq==0 || sq>sq2 ){
        id = i;
        sq = sq2;
        }
      }
    }

  //assert( sq!=0 );
  if( sq==0 ){
    Sprite n;
    n.tex = 0;
    return n;
    }

  Tempest::PixEditor p( page.p );
  Tempest::Rect r = page.rects[id];
  p.copy( r.x, r.y, px );

  if( r.w!=px.width() && r.h!=px.height() ){
    page.rects[id] = Tempest::Rect( r.x+px.width(), r.y+px.height(),
                                     r.w-px.width(), r.h-px.height() );
    } else {
    page.rects[id] = page.rects.back();
    page.rects.pop_back();
    }


  if( r.w!=px.width() )
    page.rects.push_back( Tempest::Rect( r.x+px.width(), r.y,
                                          r.w-px.width(), px.height() ) );

  if( r.h!=px.height() )
    page.rects.push_back( Tempest::Rect( r.x, r.y+px.height(),
                                          px.width(), r.h-px.height() ) );


  Sprite n;
  n.prect = Tempest::Rect( r.x, r.y, px.width(), px.height() );
  n.tex   = &this->page;

  return n;
  }

