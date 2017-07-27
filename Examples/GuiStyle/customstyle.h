#ifndef CUSTOMSTYLE_H
#define CUSTOMSTYLE_H

#include <Tempest/Style>
#include <Tempest/Color>

class CustomStyle : public Tempest::Style {
  public:
    CustomStyle(const Tempest::Color& prime);
    CustomStyle(const Tempest::Color& prime,const Tempest::Color& second);

  protected:
    void polish(Tempest::Widget& w) const;

    void draw(Tempest::Painter& p, Tempest::Button *w, Element e, const Tempest::WidgetState &st,
              const Tempest::Rect &r, const Extra &extra) const;
    void draw(Tempest::Painter& p, Tempest::Panel *w, Element e, const Tempest::WidgetState &st,
              const Tempest::Rect &r, const Extra &extra) const;

  private:
    enum {
      Light=0,
      Normal,
      Dark,
      ClCount
      };
    Tempest::Color prime [ClCount];
    Tempest::Color second[ClCount];
    Tempest::Color border;

    void mkColor(Tempest::Color* dest,const Tempest::Color& src);
  };

#endif // CUSTOMSTYLE_H
