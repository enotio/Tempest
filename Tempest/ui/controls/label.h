#ifndef LABEL_H
#define LABEL_H

#include <Tempest/Widget>

namespace Tempest {

class Label : public Widget {
  public:
    Label();

    void setFont(const Font& f);
    const Font& font() const;

    const std::u16string& text() const;
    void  setText( const std::string&    t );
    void  setText( const std::u16string& t );

  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    std::u16string txt;
    Tempest::Font  fnt;
  };

}

#endif // LABEL_H
