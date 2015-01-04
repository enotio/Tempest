#include "menu.h"

#include <Tempest/Layout>
#include <Tempest/Panel>
#include <Tempest/ListView>
#include <Tempest/ScroolWidget>

using namespace Tempest;

struct Menu::Delegate : public ListDelegate {
    Delegate( Menu& owner, const Menu::Declarator& decl ):owner(owner), decl(decl){}

    size_t  size() const{ return decl.items.size(); }

    Widget* createView(size_t position){
      return owner.createItem(decl.items[position]);
      }

    void    removeView(Widget* w, size_t /*position*/){
      w->deleteLater();
      }

    Menu& owner;
    const Menu::Declarator& decl;
  };

Menu::Overlay::~Overlay() {
  menu->overlay = 0;
  }

void Menu::Overlay::mouseDownEvent(MouseEvent &e) {
  deleteLater();
  const Point pt0 = mapToRoot(e.pos());
  const Point pt1 = owner->mapToRoot(Point());
  const Point pt = pt0-pt1;
  if(!(pt.x>=0 && pt.y>=0 && pt.x<owner->w() && pt.y<owner->h()))
    e.ignore(); else
    e.accept();
  }

Menu::Menu(const Declarator &decl):decl(decl), overlay(nullptr){
  }

Menu::~Menu() {
  close();
  }

int Menu::exec(Widget &owner) {
  return exec(owner,Tempest::Point(0,owner.h()));
  }

int Menu::exec(Widget& owner, Point &pos, bool alignWidthToOwner) {
  if( overlay )
    close();

  Widget *box = createDropList(owner,alignWidthToOwner);
  if(!box)
    return -1;
  box->setPosition( owner.mapToRoot(pos) );

  overlay = new Overlay();
  T_ASSERT( SystemAPI::instance().addOverlay( overlay )!=0 );

  overlay->owner = &owner;
  overlay->menu  = this;

  if( box->w() > overlay->w() )
    box->resize( overlay->w(), box->h() );

  if( box->h() > overlay->h() )
    box->resize( box->w(), overlay->h() );

  if( box->y()+box->h() > overlay->h() )
    box->setPosition( box->x(), overlay->h()-box->h() );
  if( box->x()+box->w() > overlay->w() )
    box->setPosition( overlay->w()-box->w(), box->y() );
  if( box->x()<0 )
    box->setPosition(0, box->y());
  if( box->y()<0 )
    box->setPosition(box->x(), 0);

  overlay->layout().add( box );

  box->setFocus(1);
  while( overlay && !Application::isQuit() ) {
    Application::processEvents();
    }
  return 0;
  }

void Menu::close() {
  if( overlay ){
    overlay->deleteLater();
    overlay = 0;
    }
  }

Widget *Menu::createDropList(Widget& owner, bool alignWidthToOwner) {
  Panel *box = new Panel();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);

  ScroolWidget *sw = new ScroolWidget();
  Widget*   list   = createItems();

  sw->centralWidget().layout().add(list);
  int wx = list->minSize().w+box->margin().xMargin();
  if(alignWidthToOwner){
    wx = std::max(owner.w(),wx);
    }
  box->resize(wx, list->h()+box->margin().yMargin());
  box->layout().add(sw);

  return box;
  }

Widget *Menu::createItems() {
  ListView* list   = new ListView(Vertical);
  list->setDelegate(Delegate(*this,decl));
  return list;
  }

Widget *Menu::createItem(const Menu::Item &decl) {
  Button* b = new Button();

  b->setText(decl.text);
  b->onClicked = decl.activated;
  b->onClicked.bind(this,&Menu::close);

  SizePolicy p = b->sizePolicy();
  p.typeH      = Preferred;
  p.maxSize.w  = SizePolicy::maxWidgetSize().w;
  b->setSizePolicy(p);

  b->setMinimumSize( b->font().textSize(b->text()).w + b->margin().xMargin(),
                     b->minSize().h );

  return b;
  }
