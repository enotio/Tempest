#ifndef TEXTMODEL_H
#define TEXTMODEL_H

#include <string>
#include <memory>

namespace Tempest {

class TextModel {
  public:
    enum {
      DefaultUndoStackLength=128
      };
    TextModel();
    virtual ~TextModel();

    const std::u16string& text() const;

    void insert(size_t pos,const char16_t  ch);
    void insert(size_t pos,const char16_t* ch);
    void append(const char16_t* ch);
    void erase (size_t pos,size_t sz);

    void setMaxUndoSteps(size_t sz);
    bool undo();

  private:
    struct Cmd;
    struct InsChar;
    struct RmChar;

    struct Cmd {
      virtual ~Cmd(){}
      virtual void redo(TextModel& m) = 0;
      virtual void undo(TextModel& m) = 0;

      std::unique_ptr<Cmd> next;
      };

    struct InsChar : Cmd {
      InsChar(size_t pos,const char16_t  c);
      InsChar(size_t pos,const char16_t* c);

      void redo(TextModel& m);
      void undo(TextModel& m);

      std::u16string val;
      size_t         pos;
      };

    struct RmChar : Cmd {
      RmChar(size_t pos,size_t sz);

      void redo(TextModel& m);
      void undo(TextModel& m);

      std::u16string val;
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

    void exec(Cmd* c);

    std::u16string data;
    UndoQueue      undoList, redoList;

    size_t               maxUndo     =DefaultUndoStackLength;
  };

}

#endif // TEXTMODEL_H
