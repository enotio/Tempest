#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <Tempest/Widget>
#include <Tempest/Font>
#include <Tempest/Timer>
#include <string>

namespace Tempest{

class LineEdit : public Tempest::Widget {
  public:
    LineEdit();
    ~LineEdit();

    enum EchoMode {
      Normal,
      NoEcho,
      Password
      };

    void setEchoMode(EchoMode m);
    EchoMode echoMode() const;

    void setFont(const Font& f);
    const Font& font() const;

    const std::u16string& text() const;
    void  setText( const std::string&    t );
    void  setText( const std::u16string& t );

    const std::u16string &hint() const;
    void  setHint( const std::u16string & str );
    void  setHint( const std::string & str    );

    Tempest::signal<const std::u16string&> onTextChanged;
    Tempest::signal<const std::u16string&> onTextEdited;
    Tempest::signal<const std::u16string&> onEditingFinished;
    Tempest::signal<const std::u16string&> onEnter;

    size_t selectionBegin();
    size_t selectionEnd();
    virtual void setSelectionBounds( size_t begin, size_t end );
    void         resetSelection();

    bool isEditable() const;
    virtual void setEditable( bool e );

    class Validator {
      public:
        virtual ~Validator(){}

        virtual void insert(std::u16string& string, size_t& cursor, size_t &ecursor, const std::u16string& data) const;
        virtual void insert(std::u16string& string, size_t& cursor, size_t &ecursor, char16_t data) const;
        virtual void erase(std::u16string& string, size_t& scursor, size_t& ecursor) const;
        virtual void assign(std::u16string& string, const std::u16string& arg) const;
      };

    class IntValidator : public Validator{
      public:
        void insert(std::u16string& string, size_t& cursor, size_t &ecursor, const std::u16string& data) const;
        void insert(std::u16string& string, size_t& cursor, size_t &ecursor, char16_t data) const;
        void erase(std::u16string& string, size_t& scursor, size_t& ecursor) const;
        void assign(std::u16string& string, const std::u16string &arg) const;
      };

    void setValidator(Validator* v);
    const Validator& validator() const;

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    void paintEvent(Tempest::PaintEvent &p);

    void keyDownEvent(Tempest::KeyEvent &e);
    void keyUpEvent(Tempest::KeyEvent &e);

    virtual void storeOldText();
    virtual void undo();
    virtual void redo();

  private:
    std::u16string txt, oldTxt, hnt;
    mutable std::unique_ptr<Validator> mvalidator;

    bool editable;
    bool anim;
    EchoMode emode=Normal;

    unsigned ctrlPressed;

    size_t sedit, eedit;
    size_t oldSedit, oldEedit;
    Tempest::Point sp, ep;
    int scroll;

    Tempest::Font  fnt;
    Tempest::Timer timer;

    bool isEdited;
    void updateSel();

    void storeText(bool);
    void setupTimer(bool);

    void animation();

    static const char16_t passChar;
  };

}

#endif // LINEEDIT_H
