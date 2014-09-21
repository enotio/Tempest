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

    virtual int     viewLengthTotal() const = 0;

    Tempest::signal<> invalidateView, updateView;
  };

template< class T >
class ArrayListDelegate : public ListDelegate {
  public:
    ArrayListDelegate( const std::vector<T>& vec ):data(vec){}

    virtual size_t size() const {
      return data.size();
      }

    virtual Widget* createView(size_t position){
      Button* b = new Button();

      SizePolicy p = b->sizePolicy();
      p.typeH      = Preferred;
      p.maxSize.w  = SizePolicy::maxWidgetSize().w;
      b->setSizePolicy(p);

      b->setText(textOf(data[position]));
      return b;
      }

    void removeView(Widget* w, size_t /*position*/){
      delete w;
      }

    int viewLengthTotal() const {
      const UiMetrics& m = Application::uiMetrics();
      return m.buttonHeight*size();
      }

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
