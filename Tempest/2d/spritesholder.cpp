#include "spritesholder.h"

#include <cassert>
#include <string>
#include <algorithm>

#include <Tempest/Sprite>
#include <Tempest/Device>


using namespace Tempest;

SpritesHolder::SpritesHolder(Tempest::TextureHolder &h):holder(h) {
  loadRq.reserve(512);
  spData.reserve(512);

  needToflush = false;
  pageSize    = std::min( h.device().caps().maxTextureSize, 2048 );
  page.reserve(2);
  addPage();
  }

SpritesHolder::~SpritesHolder() {
  for(SpriteData* l : spData){
    l->p      = Pixmap();
    l->page   = nullptr;
    l->holder = nullptr;
    }
  }

Sprite SpritesHolder::load(const char *f) {
  return load( Pixmap(f) );
  }

Sprite SpritesHolder::load(const std::string &f) {
  return load( Pixmap(f) );
  }

Sprite SpritesHolder::load(const Tempest::Pixmap &p) {
  if(p.isEmpty())
    return Sprite();
  SpriteData* data = new SpriteData();
  data->p      = p;
  data->sq     = p.width()*p.height();
  data->holder = this;

  loadRq.emplace_back(data);
  spData.emplace_back(data);

  Sprite tmp;
  tmp.data = data;
  return tmp;
  }

bool SpritesHolder::loadImpl(SpriteData& data) {
  const Pixmap& pm = data.p;
  if( pm.width()==0 || pm.height()==0 ){
    return true;
    }

  if( pm.width()  > pageSize ||
      pm.height() > pageSize ){
    page.emplace_back( new Page() );
    Page &pg = *page.back();
    pg.p = pm;

    data.page   = &pg;
    data.holder = this;
    return true;
    }

  for( size_t i=0; i<page.size(); ++i ){
    if( add(data,*page[i]) )
      return true;
    }

  addPage();
  return add(data,*page.back());
  }

void SpritesHolder::onDelete(SpritesHolder::SpriteData* data) {
  if( data->page==nullptr ){
    for(size_t i=0;i<loadRq.size();++i)
      if(loadRq[i]==data){
        loadRq[i] = loadRq.back();
        loadRq.pop_back();
        break;
        }
    }

  for(size_t i=spData.size();i>0;){
    --i;
    if(spData[i]==data){
      spData[i] = spData.back();
      spData.pop_back();
      break;
      }
    }
  if( data->page!=nullptr ){
    onRegionFreed(*data->page,data->pageRect);
    }
  delete data;
  }

void SpritesHolder::flush() {
  if( needToflush ){
    for( size_t i=0; i<page.size(); ++i ){
      if(!page[i]->needToflush)
        continue;

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

void SpritesHolder::onRegionFreed(SpritesHolder::Page& page,const Rect& /*r*/) {
  for(size_t i=0;i<spData.size();++i){
    SpriteData& data = *spData[i];

    if(data.page==&page){
      data.page     = nullptr;
      data.pageRect = Rect();
      loadRq.push_back(&data);
      }
    }

  needToflush = true;
  page.t = Texture2d();
  page.rects.resize(1);
  page.rects[0] = Tempest::Rect(0,0, page.p.width(), page.p.height() );
  }

void SpritesHolder::loadDelayd() {
  struct Grater{
    static bool cmp( const SpriteData* a, const SpriteData* b ) {
      return a->sq > b->sq;
      }
    };

  std::sort( loadRq.begin(), loadRq.end(), Grater::cmp );

  for( size_t i=0; i<loadRq.size(); ++i ){
    SpriteData& rq = *loadRq[i];
    loadImpl(rq);
    }

  loadRq.clear();
  }

void SpritesHolder::addPage() {
  page.emplace_back( new Page() );
  Page& p = *page.back();

  p.p = Tempest::Pixmap(pageSize, pageSize, true);
  p.rects.reserve(256);
  p.rects.push_back( Tempest::Rect(0,0, p.p.width(), p.p.height() ) );
  }

bool SpritesHolder::add(SpriteData& data, Page& page ) {
  needToflush = true;

  size_t id = 0, sq = 0;
  for( size_t i=0; i<page.rects.size(); ++i ){
    const Tempest::Rect r = page.rects[i];

    if( r.w>=data.p.width() && r.h>=data.p.height() ){
      size_t sq2 = r.w*r.h;
      if( sq==0 || sq>sq2 ){
        id = i;
        sq = sq2;
        }
      }
    }

  //assert( sq!=0 );
  if( sq==0 ){
    // no found space in page
    return false;
    }

  page.t = Tempest::Texture2d();
  page.needToflush = true;
  Tempest::PixEditor p( page.p );
  Tempest::Rect r = page.rects[id];
  p.copy( r.x, r.y, data.p );
  data.pageRect = Rect(r.x,r.y,data.p.width(),data.p.height());
  data.page     = &page;

  if( r.w!=data.p.width() && r.h!=data.p.height() ){
    page.rects[id] = Tempest::Rect( r.x+data.p.width(), r.y+data.p.height(),
                                    r.w-data.p.width(), r.h-data.p.height() );
    } else {
    page.rects[id] = page.rects.back();
    page.rects.pop_back();
    }


  if( r.w!=data.p.width() )
    page.rects.push_back( Tempest::Rect( r.x+data.p.width(), r.y,
                                         r.w-data.p.width(), data.p.height() ) );

  if( r.h!=data.p.height() )
    page.rects.push_back( Tempest::Rect( r.x, r.y+data.p.height(),
                                         data.p.width(), r.h-data.p.height() ) );

  return true;
  }


void SpritesHolder::SpriteData::decRef(){
  --ref;
  if(ref==0){
    if(holder)
      holder->onDelete(this); else
      delete this;
    }
  }
