#include "menu.h"

#include <Tempest/Layout>
#include <Tempest/Panel>
#include <Tempest/ListView>
#include <Tempest/ScrollWidget>

using namespace Tempest;

struct Menu::Delegate : public ListDelegate {
    Delegate( Menu& owner, const std::vector<Menu::Item>& items )
      :owner(owner), items(items){}

    size_t  size() const{ return items.size(); }

    Widget* createView(size_t position){
      return owner.createItem(items[position]);
      }

    void    removeView(Widget* w, size_t /*position*/){
      w->deleteLater();
      }

    Menu& owner;
    const std::vector<Menu::Item>& items;
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

  overlay = new Overlay();
  T_ASSERT( SystemAPI::instance().addOverlay( overlay )!=0 );

  overlay->owner = &owner;
  overlay->menu  = this;
  decl.level     = 0;
  initMenuLevels(decl);

  Widget *box = createDropList(owner,alignWidthToOwner,decl.items);
  if(!box)
    return -1;
  panels.resize(1);
  panels[0] = box;
  box->setPosition( owner.mapToRoot(pos) );
  setupMenuPanel(box);

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

Widget *Menu::createDropList( Widget& owner,
                              bool alignWidthToOwner,
                              const std::vector<Item>& items ) {
  Panel *box = new Panel();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);

  ScroolWidget *sw = new ScroolWidget();
  Widget*   list   = createItems(items);

  sw->centralWidget().layout().add(list);
  int wx = list->minSize().w+box->margin().xMargin();
  if(alignWidthToOwner){
    wx = std::max(owner.w(),wx);
    }
  box->resize(wx, list->h()+box->margin().yMargin());
  box->layout().add(sw);

  return box;
  }

Widget *Menu::createItems(const std::vector<Item>& items) {
  ListView* list = new ListView(Vertical);
  list->setDelegate(Delegate(*this,items));
  return list;
  }

Widget *Menu::createItem(const Menu::Item &decl) {
  ItemButton* b = new ItemButton(decl.items);

  b->setText(decl.text);
  b->onClicked = decl.activated;
  b->onClicked.bind(this,&Menu::close);
  b->setIcon(decl.icon);
  b->onMouseEnter.bind(this,&Menu::openSubMenu);

  SizePolicy p = b->sizePolicy();
  p.typeH      = Preferred;
  p.maxSize.w  = SizePolicy::maxWidgetSize().w;
  b->setSizePolicy(p);

  b->setMinimumSize( b->font().textSize(b->text()).w + b->margin().xMargin(),
                     b->minSize().h );

  return b;
  }

void Menu::openSubMenu(const Declarator &decl, Widget &owner) {
  if(panels.size()<=decl.level)
    panels.resize(decl.level+1);

  for(size_t i=decl.level;i<panels.size(); ++i){
    delete panels[i];
    panels[i] = nullptr;
    }

  if(!overlay || overlay->owner==nullptr || decl.items.size()==0)
    return;

  Widget *box = createDropList(*overlay->owner,false,decl.items);
  if(!box)
    return;
  panels[decl.level] = box;
  Widget* ow = decl.level>0 ? panels[decl.level-1] : nullptr;
  if(ow!=nullptr)
    box->setPosition( ow->mapToRoot(Point(ow->w(),owner.y()))); else
    box->setPosition( owner.mapToRoot(Point(owner.w(),0)) );
  setupMenuPanel(box);
  }

void Menu::setupMenuPanel(Widget* box) {
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
  }

void Menu::initMenuLevels(Menu::Declarator &decl) {
  for(Item& i:decl.items){
    i.items.level = decl.level+1;
    initMenuLevels(i.items);
    }
  }

void Menu::assign(std::u16string &s, const char *ch) {
  s.assign(ch,ch+strlen(ch));
  }

void Menu::assign(std::u16string &s, const char16_t *ch) {
  const char16_t* e = ch;
  while(*e)
    ++e;
  s.assign(ch,e);
  }

void Menu::assign(std::u16string &s, const std::string &ch) {
  s.assign(ch.begin(),ch.end());
  }

void Menu::assign(std::u16string &s, const std::u16string &ch) {
  s = ch;
  }


Menu::ItemButton::ItemButton(const Declarator &item):item(item){
  }

void Menu::ItemButton::mouseEnterEvent(MouseEvent &) {
  onMouseEnter(item,*this);
  }
