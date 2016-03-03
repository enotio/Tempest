#include "svgimage.h"

#include <Tempest/SystemAPI>

#include <cstring>

#define NANOSVG_IMPLEMENTATION
#include "thirdparty/nanosvg/nanosvg.h"

using namespace Tempest;

static float distPtSeg(float x, float y, float px, float py, float qx, float qy) {
  float pqx, pqy, dx, dy, d, t;
  pqx = qx-px;
  pqy = qy-py;
  dx = x-px;
  dy = y-py;
  d = pqx*pqx + pqy*pqy;
  t = pqx*dx + pqy*dy;

  if (d > 0)
    t /= d;
  if (t < 0)
    t = 0;
  else if (t > 1)
    t = 1;

  dx = px + t*pqx - x;
  dy = py + t*pqy - y;
  return dx*dx + dy*dy;
  }

SvgImage::SvgImage() {
  }

SvgImage::~SvgImage() {
  if(storage)
    nsvgDelete((NSVGimage*)storage);
  }

bool SvgImage::parse(const char* svg,const char* units,float dpi) {
  if(storage)
    nsvgDelete((NSVGimage*)storage);
  storage = nsvgParse((char*)svg,(char*)units,dpi);

  return storage!=nullptr;
  }

bool SvgImage::load(const char* file,const char* units,float dpi) {
  return parse(SystemAPI::loadText(file).c_str(),units,dpi);
  }

void SvgImage::paint(Painter& pt) {
  NSVGimage* image = (NSVGimage*)storage;

  const float pixelSize = 1.0f;

  for( NSVGshape* shape=image->shapes; shape; shape=shape->next) {
    uint32_t cl = shape->stroke.color;
    Color c;
    for(int i=0;i<4;++i){
      c[i] = (cl%256)/255.0;
      cl/=256;
      }

    pt.setColor( c );
    lineWidth = shape->strokeWidth;

    for( NSVGpath* path=shape->paths; path; path=path->next) {
      drawPath(pt, path->pts, path->npts, path->closed, pixelSize);
      }
    }

  return;
  lineWidth = 10;
  beginLine(   350,75);
  linePoint(pt,379,161);
  linePoint(pt,469,161);
  linePoint(pt,397,215);
  linePoint(pt,423,301);
  linePoint(pt,350,250);
  linePoint(pt,277,301);
  linePoint(pt,303,215);
  linePoint(pt,231,161);
  linePoint(pt,321,161);
  linePoint(pt,350,75);
  endLine(pt,true);
  }

void SvgImage::drawPath(Painter& pt, float* pts, int npts, char closed, float tol) {
  npts--;

  beginLine(pts[0],pts[1]);
  if( !closed ){
    sgCount++;
    lnSz++;
    }

  for(int i=0; i<npts; i+=3 ) {
    float* p = &pts[i*2];
    cubicBez(pt, p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7], tol, BezDetail);
    }

  if( closed )
    linePoint(pt, pts[0], pts[1]);
  endLine(pt,closed);
  }

void SvgImage::cubicBez( Painter& p,
                         float x1, float y1, float x2, float y2,
                         float x3, float y3, float x4, float y4,
                         float tol, int level ) {
  float x12,y12,x23,y23,x34,y34,x123,y123,x234,y234,x1234,y1234;
  float d;

  if( level<0 )
    return;

  x12   = (x1+x2)*0.5f;
  y12   = (y1+y2)*0.5f;
  x23   = (x2+x3)*0.5f;
  y23   = (y2+y3)*0.5f;
  x34   = (x3+x4)*0.5f;
  y34   = (y3+y4)*0.5f;
  x123  = (x12+x23)*0.5f;
  y123  = (y12+y23)*0.5f;
  x234  = (x23+x34)*0.5f;
  y234  = (y23+y34)*0.5f;
  x1234 = (x123+x234)*0.5f;
  y1234 = (y123+y234)*0.5f;

  d = distPtSeg(x1234, y1234, x1,y1, x4,y4);
  if( d > tol*tol ){
    cubicBez(p, x1,y1, x12,y12, x123,y123, x1234,y1234, tol, level-1);
    cubicBez(p, x1234,y1234, x234,y234, x34,y34, x4,y4, tol, level-1);
    } else {
    linePoint(p,x4,y4);
    }
  }

void SvgImage::beginLine(int x, int y) {
  lnSz    = 1;
  sgCount = 1;
  lnBuf[0]  = Point(x,y);
  lnBuf[1]  = lnBuf[0];
  lnBuf[2]  = lnBuf[0];
  loopPt[0] = Point(0,0);
  loopPt[1] = Point(0,0);
  }

void SvgImage::linePoint(Painter& p,int x, int y, bool end) {
  if(lnBuf[lnSz].x==x && lnBuf[lnSz].y==y && !end)
    return;

  lnBuf[lnSz] = Point(x,y);
  sgCount++;

  if(lnSz==2){
    int x1 = lnBuf[0].x, y1 = lnBuf[0].y,
        x2 = lnBuf[1].x, y2 = lnBuf[1].y;

    float dx = x1-x2,
          dy = y1-y2;
    float dx1 = x2 - lnBuf[2].x,
          dy1 = y2 - lnBuf[2].y;

    std::swap(dx,dy);
    std::swap(dx1,dy1);

    if(dx==0 && dy==0){
      dx = dx1;
      dy = dy1;
      } else
    if(dx1==0 && dy1==0){
      dx1 = dx;
      dy1 = dy;
      }

    float len  = sqrt(dx*dx+dy*dy);
    float len1 = sqrt(dx1*dx1+dy1*dy1);

    dx = -dx*lineWidth/len;
    dy =  dy*lineWidth/len;

    dx1 = -dx1*lineWidth/len1;
    dy1 =  dy1*lineWidth/len1;

    float bx = (dx1+dx)*0.5;
    float by = (dy1+dy)*0.5;

    float blen = sqrt(bx*bx + by*by);
    float ax = dx1-bx, ay = dy1-by;

    float alen = sqrt(ax*ax + ay*ay);
    float dlen = alen*alen/blen;

    dlen += blen;
    dx1 = dlen*bx/blen;
    dy1 = dlen*by/blen;

    if(sgCount<=3 && !end){
      loopPt[1] = Point(x,y);
      } else {
      if(sgCount>3){
        dx = oldDx;
        dy = oldDy;
        }
      p.drawTriangle(x1 -dx,y1 -dy, 0,0,
                     x1 +dx,y1 +dy, 0,0,
                     x2+dx1,y2+dy1, 0,0);

      p.drawTriangle(x2+dx1,y2+dy1, 0,0,
                     x2-dx1,y2-dy1, 0,0,
                     x1- dx,y1- dy, 0,0);
      }
    oldDx = dx1;
    oldDy = dy1;
    }

  if(lnSz<2){
    loopPt[0] = Point(x,y);
    lnSz++;
    } else {
    lnBuf[0] = lnBuf[1];
    lnBuf[1] = lnBuf[2];
    }
  }

void SvgImage::endLine(Painter& p,bool closed) {
  if(closed){
    if(sgCount<=2){
      linePoint(p,loopPt[0].x,loopPt[0].y,true);
      } else {
      linePoint(p,loopPt[0].x,loopPt[0].y);
      linePoint(p,loopPt[1].x,loopPt[1].y,true);
      }
    } else {
    linePoint(p,lnBuf[lnSz-1].x,lnBuf[lnSz-1].y,true);
    }

  lnSz    = 0;
  sgCount = 0;
  }

void SvgImage::drawLine(Painter& p, int x1, int y1, int x2, int y2) {
  float dx = x1-x2,
        dy = y1-y2;
  std::swap(dx,dy);

  float len = sqrt(dx*dx+dy*dy);
  dx = -dx*lineWidth/len;
  dy =  dy*lineWidth/len;

  p.drawTriangle(x1-dx,y1-dy, 0,0,
                 x1+dx,y1+dy, 0,0,
                 x2+dx,y2+dy, 0,0);
  p.drawTriangle(x2+dx,y2+dy, 0,0,
                 x2-dx,y2-dy, 0,0,
                 x1-dx,y1-dy, 0,0);
  }
