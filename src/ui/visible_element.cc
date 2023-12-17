#include "ui/element.hh"
#include "ui/visible_element.hh"

#include <iostream>
#include <iterator>
#include <string_view>

#include <fmt/format.h>

namespace jwezel::ui {

VisibleElement::VisibleElement(const Char &background, ui::Element *parent):
ui::Element{parent},
background_{background}
{}

} // namespace jwezel::ui
