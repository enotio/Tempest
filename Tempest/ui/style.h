#ifndef STYLE_H
#define STYLE_H

#include <Tempest/Utility>
#include <string>
#include <cstdint>

namespace Tempest {

struct Margin;
class  Painter;
class  Font;
class  Color;
class  Icon;
class  Sprite;

class  TextModel;
class  WidgetState;

class  Widget;
class  Button;
class  CheckBox;
class  Panel;
class  Label;
class  LineEdit;

class  ScrollBar;

class Style {
  public:
    Style();
    Style(const Style* parent);
    virtual ~Style();

    struct Extra {
      public:
        Extra(const Widget&   owner);
        Extra(const Button&   owner);
        Extra(const Label&    owner);
        Extra(const LineEdit& owner);

        const Margin&         margin;
        const Icon&           icon;
        const Font&           font;
        const Color&          fontColor;

      private:
        static const Tempest::Margin emptyMargin;
        static const Tempest::Icon   emptyIcon;
        static const Tempest::Font   emptyFont;
        static const Tempest::Color  emptyColor;
      };

    enum Element {
      E_Background,
      E_ArrowUp,
      E_ArrowDown,
      E_ArrowLeft,
      E_ArrowRight,
      E_CentralButton
      };

    enum TextElement {
      TE_ButtonTitle,
      TE_CheckboxTitle,
      TE_LabelTitle,
      TE_LineEditContent
      };

    void setParent(const Style* stl);

    // common
    virtual void draw(Painter& p, Widget*   w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Panel *   w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Button*   w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, CheckBox* w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Label*    w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, LineEdit* w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;

    // complex
    virtual void draw(Painter& p, ScrollBar* w, Element e, const WidgetState& st, const Rect& r, const Extra& extra) const;

    // text
    virtual void draw(Painter& p, const std::u16string& text, TextElement e, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, const TextModel&      text, TextElement e, const WidgetState& st, const Rect& r, const Extra& extra) const;

  protected:
    virtual void polish  (Widget& w) const;
    virtual void unpolish(Widget& w) const;

    static const Tempest::Sprite& iconSprite(const Icon& icon,const WidgetState &st, const Rect &r);

  private:
    mutable uint32_t polished=0;
    mutable int32_t  counter =0;
    const   Style*   parent  =nullptr;

    void    addRef() const { counter++;                             }
    void    decRef() const { counter--; if(counter==0) delete this; }

    static void drawCursor(Painter &p, bool emptySel, const WidgetState &st, int x1, int x2, int h, bool animState);

  friend class Widget;
  };

}

#endif // STYLE_H
