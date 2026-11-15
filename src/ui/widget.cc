#include "container.hh"
#include "element.hh"
#include "root.hh"
#include "widget.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

#include <exception>

namespace jwezel::ui {

namespace {

// Area on the display; an area that is left to the layout stays unassigned
auto displayArea(const Rectangle &area, Container &parent) -> Rectangle {
  return area == RectangleDefault? area: area + parent.root()->origin();
}

} // namespace

Widget::Widget(const Rectangle &area, const Char &background, Container &parent):
TextElement{
  parent.root()->host().surface(),
  displayArea(area, parent),
  background
},
ui::Element{&parent}
{
  parent.add(*this);
  if (auto *root{parent.root()}; !root->focus()) {
    root->focus(this);
  }
}

Widget::~Widget() {
  try {
    root()->forget(*this);
  } catch (const std::exception &) { // no root to tell
  }
}

void Widget::move(const Rectangle &area) {
  TextElement::move(area + root()->origin());
}

auto Widget::inputWindow() -> jwezel::Window * {
  return &root()->host();
}

auto Widget::bounds() -> Rectangle {
  return TextElement::area() - root()->origin();
}

auto Widget::moveEvent(const Rectangle &area) -> bool {
  TextElement::moveEvent(area);
  render();
  return true;
}

} // namespace jwezel::ui
