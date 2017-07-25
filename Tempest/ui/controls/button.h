#ifndef BUTTON_H
#define BUTTON_H

#include <Tempest/Widget>
#include <Tempest/Shortcut>
#include <Tempest/Sprite>
#include <Tempest/Font>
#include <Tempest/Icon>

#include <ctime>

namespace Tempest {

class Menu;

/** \addtogroup GUI
 *  @{
 */
class Button : public Tempest::Widget {
  public:
    Button();
    ~Button();

    using Type=Tempest::WidgetState::ButtonType;

    static constexpr Type T_PushButton=Type::T_PushButton;
    static constexpr Type T_ToolButton=Type::T_ToolButton;
    static constexpr Type T_FlatButton=Type::T_FlatButton;

    Tempest::signal<> onClicked;

    void  setIcon(const Tempest::Sprite& s);
    void  setIcon(const Tempest::Icon&   s);
    const Tempest::Icon& icon() const;

    const Tempest::Shortcut& shortcut() const;
    void  setShortcut( const Tempest::Shortcut & sc );

    const std::u16string& text() const;
    void  setText(const std::u16string &t );
    void  setText( const std::string& t   );

    const std::u16string &hint() const;
    void  setHint( const std::u16string & str );
    void  setHint( const std::string & str    );

    const Tempest::Font& font() const;
    void  setFont( const Tempest::Font& f );

    void  setTextColor(const Tempest::Color& color);
    const Tempest::Color& textColor() const;

    void  setButtonType(Type t);
    Type  buttonType() const;

    void  setMenu(Tempest::Menu* menu);
    Tempest::Menu* menu() const;

    void showMenu();

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    void mouseEnterEvent(Tempest::MouseEvent& e);
    void mouseLeaveEvent(Tempest::MouseEvent& e);

    void focusEvent(Tempest::FocusEvent& e);

    void paintEvent( Tempest::PaintEvent &p);

    void gestureEvent(Tempest::AbstractGestureEvent &e);

    void keyUpEvent(Tempest::KeyEvent &e);

    void onShortcut();
    virtual void emitClick();

    void setPressed(bool p);
    bool isPressed() const;

    void setWidgetState(const WidgetState& s);

  private:
    std::u16string    txt, hnt;

    Tempest::Shortcut hotKey;
    Tempest::Icon     icn;

    Tempest::Font     fnt;
    Tempest::Color    fntColor;

    std::unique_ptr<Tempest::Menu> btnMenu;
  };
/** @}*/

}

#endif // BUTTON_H
