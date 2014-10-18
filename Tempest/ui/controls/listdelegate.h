#ifndef LISTDELEGATE_H
#define LISTDELEGATE_H

#include <cstddef>
#include <vector>

#include <Tempest/Button>
#include <Tempest/Application>

namespace Tempest{

class Widget;

class ListDelegate {
  public:
    virtual ~ListDelegate() = default;

    virtual size_t  size() const = 0;

    virtual Widget* createView(size_t position           ) = 0;
    virtual void    removeView(Widget* w, size_t position) = 0;

    Tempest::signal<> invalidateView, updateView;
    Tempest::signal<size_t> onItemSelected;
  };

template< class T >
class ArrayListDelegate : public ListDelegate {
  public:
    ArrayListDelegate( const std::vector<T>& vec ):data(vec){}

    virtual size_t size() const {
      return data.size();
      }

    virtual Widget* createView(size_t position){
      ListItem* b = new ListItem(position);
      b->clicked.bind(onItemSelected);

      SizePolicy p = b->sizePolicy();
      p.typeH      = Preferred;
      p.maxSize.w  = SizePolicy::maxWidgetSize().w;
      b->setSizePolicy(p);

      b->setText(textOf(data[position]));
      return b;
      }

    void removeView(Widget* w, size_t /*position*/){
      w->deleteLater();
      }

  protected:
    struct ListItem: Button{
      ListItem(size_t id):id(id){}

      const size_t id;
      Tempest::signal<size_t> clicked;

      void emitClick(){
        Button::emitClick();
        clicked(id);
        }
      };

  private:
    const std::vector<T>& data;

    const std::string    textOf( const std::string& s ){
      return s;
      }
    const std::u16string textOf( const std::u16string& s ){
      return s;
      }

    template< class E >
    const std::string textOf( const E& s ){
      return std::to_string(s);
      }
  };
}

#endif // LISTDELEGATE_H
