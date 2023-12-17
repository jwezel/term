#include "ui/widget.hh"
#include "geometry.hh"
#include "ui/window.hh"

#include <iostream>
#include <iterator>
#include <string_view>

#include <fmt/format.h>

namespace jwezel::ui {

Widget::Widget(const Char &background, const Rectangle &area, ui::Element *parent):
VisibleElement{background, parent},
text_{background, area.size()}
{
  auto *window{dynamic_cast<Window *>(parent->window())};
  if (window) {
    window->addElement(this, 0);
  }
}

void Widget::write(const Vector &position, const string_view &str) {
  text_.patch(Text(str), position);
}

void Widget::write(const Vector &position, const Text &txt_) {
  text_.patch(txt_, position);
}

void Widget::fill(const Char &fillChar, const Rectangle &area) {
  text_.fill(fillChar, area);
}

auto Widget::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Rectangle {
  return text_.line(line, strength, dash, roundedCorners);
}

auto Widget::box(const Box &box) -> vector<Rectangle> {
  return text_.box(box);
}

auto Widget::area() const -> Rectangle {
  return Rectangle{position_, position_ + text_.size()};
}

void Widget::move(const Rectangle &area) {
  position_ = area.position();
  text_.resize(area.size(), background());
}

} // namespace jwezel::ui
