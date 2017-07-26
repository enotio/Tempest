#ifndef TEXTMODEL_H
#define TEXTMODEL_H

#include <Tempest/Font>
#include <Tempest/Utility>
#include <Tempest/WidgetState>

#include <string>
#include <memory>

namespace Tempest {

class Font;
class Painter;

class AbstractTextModel {
  public:
    enum {
      DefaultUndoStackLength=128
      };
    AbstractTextModel();
    virtual ~AbstractTextModel();

    virtual const std::u16string& text()  const = 0;
    virtual size_t                size()  const;
    inline  bool                  empty() const { return size()==0; }

    void        insert (size_t pos,const char16_t        ch );
    void        insert (size_t pos,const char16_t*       str);
    void        insert (size_t pos,const std::u16string& str);
    void        append (const char16_t* str);
    void        erase  (size_t pos,size_t sz);
    void        clear  ();
    void        replace(size_t pos,size_t sz,const char16_t*       str);
    void        replace(size_t pos,size_t sz,const std::u16string& str);
    void        assign (const char16_t*       str);
    void        assign (const std::u16string& str);

    void        setMaxUndoSteps(size_t sz);
    bool        undo();
    bool        redo();

    size_t      availableUndoSteps() const;
    size_t      availableRedoSteps() const;

    void        clearSteps();
    void        clearChains();

    const Font& defaultFont() const;
    void        setDefaultFont(const Font& font);

    void        setViewport(const Size& sz);
    const Size& viewport() const;

    size_t      selectionBegin() const;
    size_t      selectionEnd()   const;

    void        setSelectionBounds( size_t both ){ setSelectionBounds(both,both); }
    void        setSelectionBounds( size_t begin, size_t end );

    size_t      cursorForPosition(const Point& pos, const WidgetState::EchoMode m) const;
    Point       positionForCursor(size_t i, const WidgetState::EchoMode m) const;
    void        paint(Tempest::Painter& p, const Color &fcolor, const Color &sel, const WidgetState::EchoMode echoMode) const;

  protected:
    virtual void rawInsert(size_t pos,const char16_t        str)       = 0;
    virtual void rawInsert(size_t pos,const std::u16string& str)       = 0;
    virtual void rawErase (size_t pos, size_t count, char16_t *outbuf) = 0;

  private:
    struct Sel {
      size_t b=0;
      size_t e=0;
      };

    struct Cmd {
      virtual ~Cmd(){}
      virtual void redo(AbstractTextModel& m) = 0;
      virtual void undo(AbstractTextModel& m) = 0;

      Sel                  sel;
      std::unique_ptr<Cmd> next;
      };

    struct InsChar : Cmd {
      InsChar(size_t pos,const char16_t        c);
      InsChar(size_t pos,const char16_t*       c);
      InsChar(size_t pos,const std::u16string& c);

      void redo(AbstractTextModel& m) override;
      void undo(AbstractTextModel& m) override;

      bool patch(AbstractTextModel &m,size_t pos,const char16_t c);

      std::u16string val;
      size_t         pos;
      };

    struct RmChar : Cmd {
      RmChar(size_t pos,size_t sz);

      void redo(AbstractTextModel& m) override;
      void undo(AbstractTextModel& m) override;

      bool patch(AbstractTextModel &m,size_t pos,size_t sz);

      std::u16string val;
      size_t         pos;
      };

    struct Replace : Cmd {
      Replace(size_t pos,size_t sz,const char16_t*       str);
      Replace(size_t pos,size_t sz,const std::u16string& str);

      void redo(AbstractTextModel& m) override;
      void undo(AbstractTextModel& m) override;

      std::u16string val;
      std::u16string old;
      size_t         pos;
      };

    struct UndoQueue {
      std::unique_ptr<Cmd> list;
      size_t               size=0;

      void push(Cmd* c);
      Cmd* pop();

      void cut(size_t sz);
      void clear();
      };

    enum CharCategory {
       CC_Normal,
       CC_Space,
       CC_LineBreak
       };

    static CharCategory charCategory(const char16_t c);

    void exec(InsChar* c);
    void exec(RmChar*  c);
    bool exec(Cmd*     c);

    InsChar*  insChain=nullptr;
    RmChar*   rmChain =nullptr;
    size_t    maxUndo=DefaultUndoStackLength;
    UndoQueue undoList, redoList;

    Font      fnt;
    Size      sz;

    Sel       selection;
    static const char16_t passChar;
  };

class TextModel:public AbstractTextModel {
  public:
    const std::u16string& text() const override;

  protected:
    void rawInsert(size_t pos,const char16_t        str)     override;
    void rawInsert(size_t pos,const std::u16string& str)     override;
    void rawErase (size_t pos,size_t count,char16_t *outbuf) override;

  private:
    std::u16string data;
  };
}

#endif // TEXTMODEL_H
