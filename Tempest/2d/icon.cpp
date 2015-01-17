#include "icon.h"

using namespace Tempest;

Icon::Icon() {
  }

Icon::Icon(const Pixmap &pm, SpritesHolder &h) : normal(h.load(pm)) {
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

  disabled = h.load(d);
  }


