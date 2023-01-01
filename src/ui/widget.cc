#include "element.hh"
#include "text.hh"
#include "update.hh"
#include "widget.hh"

#include <iostream>
#include <iterator>
#include <string_view>

#include <fmt/format.h>

using namespace jwezel::ui;

Widget::Widget(Element *parent, const Char &background, const Vector &size):
Element{parent, Vertical, size},
background_{background}
{
  needsDrawing_ = true;
}

jwezel::Updates Widget::draw() {
  Updates result{Element::draw()};
  text_.extend(size());
  text_.fill(background_);
  // fmt::print(stderr, "Draw element {}: \n", fmt::ptr(this));
  result.emplace_back(area().position(), text_);
  return result;
}
