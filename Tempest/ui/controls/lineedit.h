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
    void   setSelectionBounds( size_t begin, size_t end );

    bool isEditable() const;
    void setEditable( bool e );

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

    bool editable;
    bool anim;
    unsigned ctrlPressed;

    size_t sedit, eedit;
    size_t oldSedit, oldEedit;
    Tempest::Point sp, ep;
    int scrool;

    Tempest::Font   font;
    Tempest::Timer  timer;

    bool isEdited;
    void updateSel();

    void storeText(bool);
    void setupTimer(bool);

    void animation();
  };

}

#endif // LINEEDIT_H
