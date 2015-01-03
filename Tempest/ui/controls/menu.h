#ifndef MENU_H
#define MENU_H

#include <Tempest/Widget>
#include <Tempest/Sprite>
#include <Tempest/Window>

namespace Tempest {

class Menu : public slot {
  private:
    struct Delegate;

  protected:
    struct Item{
      std::u16string    text;
      Tempest::Sprite   icon;
      Tempest::signal<> activated;
      };

  public:
    struct Declarator;

    Menu(const Declarator& decl);
    virtual ~Menu();

    int  exec(Widget& owner);
    int  exec(Widget& owner,Point& pos,bool alignWidthToOwner = true);
    void close();

    struct Declarator{
      template<class Str, class ... Args>
      Declarator& item(const Str& text, Args... a){
        items.emplace_back();

        Item& it = items.back();
        it.text.assign(std::begin(text),std::end(text));
        it.activated.bind(a...);

        return *this;
        }

      template<class Str, class ... Args>
      Declarator& item(const Tempest::Sprite& icon, const Str& text, Args... a){
        items.emplace_back();

        Item& it = items.back();
        it.text.assign(std::begin(text),std::end(text));
        it.icon = icon;
        it.activated.bind(a...);

        return *this;
        }

      private:
        std::vector<Item> items;

      friend struct Delegate;
      };

  protected:
    virtual Tempest::Widget *createDropList(Widget &owner, bool alignWidthToOwner);
    virtual Widget* createItems();
    virtual Widget* createItem(const Item& decl);

  private:
    struct Overlay:Tempest::WindowOverlay {
      ~Overlay();
      void mouseDownEvent( Tempest::MouseEvent& e );

      Widget* owner;
      Menu  * menu;
      };

    Overlay * overlay;
    Declarator decl;
  };
}

#endif // MENU_H
