#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <Tempest/Widget>
#include <Tempest/Font>
#include <Tempest/TextModel>

#include <string>

namespace Tempest{

/** \addtogroup GUI
 *  @{
 */
class LineEdit : public Tempest::Widget {
  public:
    LineEdit();
    ~LineEdit();

    using EchoMode=WidgetState::EchoMode;

    static constexpr EchoMode Normal  =WidgetState::Normal;
    static constexpr EchoMode NoEcho  =WidgetState::NoEcho;
    static constexpr EchoMode Password=WidgetState::Password;

    void setEchoMode(EchoMode m);
    EchoMode echoMode() const;

    void setTabChangesFocus(bool ch);
    bool tabChangesFocus() const;

    void setFont(const Font& f);
    const Font& font() const;

    const std::u16string& text() const;
    void  setText( const std::string&    t );
    void  setText( const std::u16string& t );

    void         setTextColor(const Color& c);
    const Color& textColor() const;

    const std::u16string &hint() const;
    void  setHint( const std::u16string & str );
    void  setHint( const std::string & str    );

    Tempest::signal<const std::u16string&> onTextChanged;
    Tempest::signal<const std::u16string&> onTextEdited;
    Tempest::signal<const std::u16string&> onEditingFinished;
    Tempest::signal<const std::u16string&> onEnter;

    size_t       selectionBegin() const;
    size_t       selectionEnd()   const;

    virtual void setSelectionBounds( size_t begin, size_t end );
    void         resetSelection();

    bool         isEditable() const;
    virtual void setEditable( bool e );

    class Validator {
      public:
        virtual ~Validator(){}

        virtual void insert(TextModel& string, size_t& cursor, size_t &ecursor, const std::u16string& data) const;
        virtual void insert(TextModel& string, size_t& cursor, size_t &ecursor, char16_t data) const;
        virtual void erase (TextModel& string, size_t& scursor, size_t& ecursor) const;
        virtual void assign(TextModel& string, const std::u16string& arg) const;
      };

    class IntValidator : public Validator {
      public:
        void insert(TextModel& string, size_t& cursor, size_t &ecursor, const std::u16string& data) const;
        void insert(TextModel& string, size_t& cursor, size_t &ecursor, char16_t data) const;
        void erase (TextModel& string, size_t& scursor, size_t& ecursor) const;
        void assign(TextModel& string, const std::u16string &arg) const;
      };

    void setValidator(Validator* v);
    const Validator& validator() const;

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    void paintEvent(Tempest::PaintEvent &p);
    void resizeEvent(Tempest::SizeEvent& e);

    void keyDownEvent(Tempest::KeyEvent &e);
    void keyUpEvent(Tempest::KeyEvent &e);

    void focusEvent(Tempest::FocusEvent& e);

    virtual void undo();
    virtual void redo();
    virtual void drawCursor(Painter& p, int x1, int x2, bool animState);

  private:
    TextModel      txt;
    std::u16string hnt;
    mutable std::unique_ptr<Validator> mvalidator;

    Color tColor;
    bool  tabChFocus=0;

    size_t pressPos =0;
    size_t sedit    =0;
    size_t eedit    =0;

    //Tempest::Point sp, ep;

    int           scroll=0;

    bool isEdited;
    void updateSel();

    void storeText();
    void setWidgetState(const WidgetState& s);

    static const int      cursorFlashTime;
    static const char16_t passChar;
  };
/** @}*/

}

#endif // LINEEDIT_H
