#pragma once

#include "layout_element.hh"
#include "text.hh"

namespace jwezel::ui {

struct Widget: public LayoutElement
{
  Widget(struct Container *parent=0, const Char &background=Space, const std::string_view &text="");

  virtual ~Widget() = default;

  virtual void draw();

  Char background;
  Text text;
};

} // namespace
