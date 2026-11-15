#pragma once

#include "ui/element.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

namespace jwezel::ui {

struct Widget: TextElement, ui::Element
{
  explicit Widget(const Rectangle &area, const Char &background, Container &parent);

  Widget(const Widget &) = delete;

  Widget(Widget &&) = delete;

  auto operator=(const Widget &) -> Widget & = delete;

  auto operator=(Widget &&) -> Widget & = delete;

  ~Widget() override = default;
};

} // namespace jwezel::ui
