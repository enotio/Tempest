#include "stackedwidget.h"

#include <Tempest/Layout>

using namespace Tempest;

struct StackedWidget::Layout : public Tempest::Layout {
  void applyLayout(){
    const Margin& m = owner()->margin();
    Rect r = Rect(m.left,m.top, owner()->w()-m.xMargin(), owner()->h()-m.yMargin());
    for(Widget* w:widgets()){
      w->setGeometry(r);
      }
    }
  };

const size_t StackedWidget::noPage = size_t(-1);

StackedWidget::StackedWidget() : current(-1) {
  Widget::setLayout(new Layout());
  }

StackedWidget::~StackedWidget() {
  for(Widget *w:pages)
    delete w;
  }

void StackedWidget::addPage(Widget *w) {
  T_ASSERT(w!=nullptr);
  pages.push_back(w);
  if(pages.size()==1)
    setCurrentPage(0);
  }

void StackedWidget::delPage(Widget *w) {
  delete takePage(w);
  }

Widget *StackedWidget::takePage(Widget *w) {
  for(size_t i=0; i<pages.size(); ++i)
    if(pages[i]==w){
      size_t page = current;
      if( page>i )
        --page; else
      if( page==i )
        page = 0;

      pages.resize(std::remove(pages.begin(),pages.end(),w)-pages.begin());
      setCurrentPage(page);
      return w;
      }

  return w;
  }

Widget *StackedWidget::page(size_t num) {
  return pages[num];
  }

const Widget *StackedWidget::page(size_t num) const {
  return pages[num];
  }

size_t StackedWidget::pagesCount() const {
  return pages.size();
  }

size_t StackedWidget::currentPage() const {
  return current;
  }

void StackedWidget::setCurrentPage(size_t num) {
  if(pages.size()==0){
    current = -1;
    return;
    }

  size_t c = std::min(num,pagesCount()-1);
  if(c==current)
    return;

  current = c;
  Tempest::Layout& lay = layout();
  while(lay.widgets().size())
    lay.take(lay.widgets().back());

  lay.add(pages[current]);
  onPageChanged(c);
  }

void StackedWidget::setLayout(Tempest::Layout *) {
  T_WARNING_X(0,"StackedWidget::setLayout is not available");
  }

