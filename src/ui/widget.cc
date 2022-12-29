#include "fmt/format.h"

#include "element.hh"
#include "text.hh"
#include "update.hh"
#include "widget.hh"
#include <iostream>
#include <iterator>
#include <string_view>

using namespace jwezel::ui;

Widget::Widget(Element *parent, const Char &background):
Element{parent},
background_{background}
{
  needsDrawing_ = true;
}

jwezel::Updates Widget::draw() {
  fmt::print(stderr, "Draw element {}\n", fmt::ptr(this));
  Updates result{Element::draw()};
  text_.fill(background_);
  result.emplace_back(area_.position(), text_);
  return result;
}
