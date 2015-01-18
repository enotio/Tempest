#ifndef MENU_H
#define MENU_H

#include <Tempest/Widget>
#include <Tempest/Sprite>
#include <Tempest/Window>
#include <Tempest/Icon>
#include <Tempest/Button>

namespace Tempest {

class Menu : public slot {
  private:
    struct Delegate;

  protected:
    struct Item;

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
        assign(it.text,text);
        bind(it,a...);

        return *this;
        }

      template<class Str, class ... Args>
      Declarator& item(const Tempest::Icon& icon, const Str& text, Args... a){
        items.emplace_back();

        Item& it = items.back();
        assign(it.text,text);
        it.icon = icon;
        bind(it,a...);

        return *this;
        }

      Declarator& operator [](const Declarator& d){
        T_ASSERT_X(items.size()>0,"no item found");
        items.back().items = d;
        return *this;
        }

      Declarator& operator [](Declarator&& d){
        T_ASSERT_X(items.size()>0,"no item found");
        items.back().items = std::move(d);
        return *this;
        }

      private:
        std::vector<Item> items;
        size_t            level = 0;

        void bind(Item&){}

        template<class ... Args>
        void bind(Item& it, Args... a){
          it.activated.bind(a...);
          }

      friend class Menu;
      };

  protected:
    struct Item{
      std::u16string    text;
      Tempest::Icon     icon;
      Tempest::signal<> activated;
      Declarator        items;
      };

    virtual Tempest::Widget *createDropList( Widget &owner,
                                             bool alignWidthToOwner,
                                             const std::vector<Item> &items );
    virtual Widget* createItems(const std::vector<Item> &items);
    virtual Widget* createItem(const Item& decl);

    struct ItemButton:Tempest::Button {
      ItemButton(const Declarator& item);
      void mouseEnterEvent(MouseEvent& e);

      Tempest::signal<const Declarator&,Widget&> onMouseEnter;
      private:
        const Declarator& item;
      };

    void openSubMenu(const Declarator &items, Widget& root);
    void setupMenuPanel(Widget *box);

  private:
    struct Overlay:Tempest::WindowOverlay {
      ~Overlay();
      void mouseDownEvent( Tempest::MouseEvent& e );

      Widget* owner;
      Menu  * menu;
      };

    void initMenuLevels(Declarator& decl);

    std::vector<Widget*> panels;
    Overlay * overlay;
    Declarator decl;

    static void assign(std::u16string& s, const char*     ch);
    static void assign(std::u16string& s, const char16_t* ch);

    static void assign(std::u16string& s, const std::string& ch);
    static void assign(std::u16string& s, const std::u16string& ch);
  };
}

#endif // MENU_H
