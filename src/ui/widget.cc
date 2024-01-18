#include "container.hh"
#include "element.hh"
#include "term/surface.hh"
#include "widget.hh"
#include "window.hh"

#include <term/geometry.hh>


namespace jwezel::ui {

Widget::Widget(const Rectangle &area, const Char &background, Container *parent):
TextElement{
  parent->window()->surface(),
  area,
  background
},
ui::Element{dynamic_cast<Container *>(parent)}
{}

} // namespace jwezel::ui
