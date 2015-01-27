#ifndef LISTDELEGATE_H
#define LISTDELEGATE_H

#include <cstddef>
#include <vector>

#include <Tempest/Button>
#include <Tempest/Application>

namespace Tempest{

class Widget;

class ListDelegate : public slot {
  public:
    enum Role{
      R_Default,
      R_ListBoxView,
      R_ListItem,
      R_Count
      };

    virtual ~ListDelegate(){}

    virtual size_t  size() const = 0;

    virtual Widget* createView(size_t position, Role /*role*/){
      return createView(position);
      }
    virtual Widget* createView( size_t position            ) = 0;
    virtual void    removeView( Widget* w, size_t position ) = 0;
    virtual Widget* update    ( Widget* w, size_t position ){
      removeView(w,position);
      return createView(position,R_Default);
      }

    Tempest::signal<> invalidateView, updateView;
    Tempest::signal<size_t>         onItemSelected;
    Tempest::signal<size_t,Widget*> onItemViewSelected;
  };

template< class T, class VT, class Ctrl >
class AbstractListDelegate : public ListDelegate {
  public:
    AbstractListDelegate( const VT& vec ):data(vec){}

    virtual size_t size() const {
      return data.size();
      }

    virtual Widget* createView(size_t position){
      ListItem<Ctrl>* b = new ListItem<Ctrl>(position);
      b->clicked.bind(this,&AbstractListDelegate<T,VT,Ctrl>::implItemSelected);
      initializeItem(b,data[position]);
      return b;
      }

    void removeView(Widget* w, size_t /*position*/){
      w->deleteLater();
      }

  protected:
    const VT& data;

    void initializeItem(Ctrl* c, const T& data){
      SizePolicy p = c->sizePolicy();
      p.typeH      = Preferred;
      p.maxSize.w  = SizePolicy::maxWidgetSize().w;
      c->setSizePolicy(p);

      c->setText(textOf(data));
      c->setMinimumSize( c->font().textSize(c->text()).w + c->margin().xMargin(),
                         c->minSize().h );
      }

    template<class C>
    struct ListItem: C {
      ListItem(size_t id):id(id){}

      const size_t id;
      Tempest::signal<size_t,Widget*> clicked;

      void emitClick(){
        clicked(id,this);
        }
      };

    virtual const std::u16string textOf( const T& t ) const = 0;

  private:
    void implItemSelected(size_t item, Widget* itemView){
      onItemSelected(item);
      onItemViewSelected(item,itemView);
      }
  };

template< class T, class Ctrl=Button >
class ArrayListDelegate : public AbstractListDelegate<T,std::vector<T>,Ctrl> {
  public:
    ArrayListDelegate( const std::vector<T>& vec ):
      AbstractListDelegate<T,std::vector<T>,Ctrl>(vec){}

  private:
    virtual const std::u16string textOf( const T& t ) const {
      return std::move(implTextOf(t));
      }

    static std::u16string implTextOf( const std::string& s ){
      return SystemAPI::toUtf16(s);
      }

    static std::u16string implTextOf( const std::u16string& s ){
      return s;
      }

    template< class E >
    static std::u16string implTextOf( const E& s ){
      return SystemAPI::toUtf16(std::to_string(s));
      }
  };
}

#endif // LISTDELEGATE_H
