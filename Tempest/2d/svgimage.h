#ifndef SVGIMAGE_H
#define SVGIMAGE_H

#include <Tempest/Painter>

namespace Tempest{

class SvgImage {
  enum constant{
    BezDetail = 13
    };
  public:
    SvgImage();
    virtual ~SvgImage();

    bool parse(const char* svg, const char* units="px", float dpi=96);
    bool load(const char* file, const char* units="px", float dpi=96);

    void paint(Tempest::Painter& p);

  private:
    void* storage=nullptr;
    Point bezPrev;
    Point lnBuf[3], loopPt[2];
    int   lnSz=0, sgCount=0;
    float oldDx,oldDy;
    float lineWidth=1;

    void drawPath(Painter& pt, float* pts, int npts, char closed, float tol);
    void cubicBez( Painter& p,
                   float x1, float y1, float x2, float y2,
                   float x3, float y3, float x4, float y4,
                   float tol, int level );

    void beginLine(int x, int y);
    void linePoint(Painter& p, int x, int y, bool end=false);
    void endLine(Painter& p, bool closed);

    void drawLine(Painter& p, int x1,int y1,int x2,int y2);
  };

}

#endif // SVGIMAGE_H
