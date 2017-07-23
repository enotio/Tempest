#include "icon.h"

using namespace Tempest;

Icon::Icon() {
  }

Icon::Icon(const Pixmap &pm, SpritesHolder &h) {
  set(ST_Normal,h.load(pm));

  Pixmap d = Pixmap(pm.width(),pm.height(),pm.hasAlpha());
  const uint8_t* p = pm.const_data();

  if(pm.hasAlpha()){
    for(int r=0; r<pm.height(); ++r)
      for(int i=0; i<pm.width(); ++i){
        //0.299, 0.587, 0.114
        uint8_t cl = uint8_t(p[0]*0.299 + p[1]*0.587 + p[2]*0.114);
        d.set(i,r,Pixmap::Pixel{cl,cl,cl,p[3]});
        p += 4;
        }
    } else {
    for(int r=0; r<pm.height(); ++r)
      for(int i=0; i<pm.width(); ++i){
        //0.299, 0.587, 0.114
        uint8_t cl = uint8_t(p[0]*0.299 + p[1]*0.587 + p[2]*0.114);
        d.set(i,r,Pixmap::Pixel{cl,cl,cl,255});
        p += 3;
        }
    }

  set(ST_Disabled,h.load(d));
  }

const Sprite &Icon::sprite(int w,int h,Icon::State st) const {
  return val[st].sprite(w,h);
  }

void Icon::set(Icon::State st,const Sprite &s) {
  return val[st].set(s);
  }

const Sprite& Icon::SzArray::sprite(int w, int h) const {
  if(emplace.w()==w && emplace.h()==h)
    return emplace;

  const Sprite* ret=&emplace;
  for(auto& i:data)
    if( i.w()<=w && i.h()<=h )
      if( ret->w()<i.w() || (ret->w()==i.w() && ret->h()<i.h()) )
        ret=&i;
  if( ret->w()<=w && ret->h()<=h )
    return *ret;

  static Sprite s;
  return s;
  }

void Icon::SzArray::set(const Sprite &s) {
  if( emplace.size().isEmpty() || emplace.size()==s.size() ) {
    emplace=s;
    return;
    }

  for(auto& i:data)
    if(i.size()==s.size()){
      i=s;
      return;
      }
  data.emplace_back(s);
  }
