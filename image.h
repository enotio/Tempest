#ifndef IMAGE_H
#define IMAGE_H

#include <Tempest/Widget>

namespace Tempest{

template< class InTexture = Tempest::Bind::UserTexture >
class Image : public Widget {
  public:
    typedef InTexture Texture;
    Texture texture;

    void mouseDownEvent(MouseEvent &e){
      Widget::mouseDownEvent(e);
      e.accept();
      }

    void mouseUpEvent(MouseEvent &e){
      Widget::mouseUpEvent(e);
      e.accept();
      }

    void mouseMoveEvent(MouseEvent &e){
      Widget::mouseMoveEvent(e);
      // e.ignore();
      }

  protected:
    void paintEvent( PaintEvent &pe ){
      PainterDevice & p = pe.painter;

      p.setTexture( texture );
      p.drawRect(0, 0, this->w(), this->h() );
      p.unsetTexture();

      paintNested(pe);

      }

  };

}

#endif // IMAGE_H
