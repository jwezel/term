#include "container.hh"
#include "element.hh"
#include "widget.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

namespace jwezel::ui {

Widget::Widget(const Rectangle &area, const Char &background, Container &parent):
TextElement{
  parent.window()->surface(),
  area + parent.window()->area().position(),
  background
},
ui::Element{&parent}
{
  parent.add(*this);
  if (auto *window{parent.window()}; !window->focus()) {
    window->focus(this);
  }
}

void Widget::move(const Rectangle &area) {
  TextElement::move(area + window()->area().position());
}

auto Widget::inputWindow() -> jwezel::Window * {
  return window();
}

auto Widget::bounds() -> Rectangle {
  return TextElement::area() - window()->area().position();
}

auto Widget::moveEvent(const Rectangle &area) -> bool {
  TextElement::moveEvent(area);
  render();
  return true;
}

} // namespace jwezel::ui
