#include "ui.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <algorithm>
#include <memory>

namespace jwezel::ui {

namespace {

auto DefaultSize(Ui &ui, const Rectangle &area) {
  if (area == RectangleDefault) {
    return Rectangle{Vector{0, 0}, ui.terminal().display().maxSize() * 2 / 3};
  }
  return area;
}

} // namespace

Window::Window(
  Ui &ui,
  const Char &background,
  const Rectangle &area,
  const Text &title,
  const Decoration *decoration,
  const ColorStyle *colors
):
jwezel::Window{&ui.terminal(), DefaultSize(ui, area), background},
Root{static_cast<jwezel::Window &>(*this)},
title_{title},
decoration_{decoration},
colors_{colors},
state_{ui.terminal().focus() == static_cast<jwezel::Window *>(this)? DecorationState::active: DecorationState::inactive}
{
  decorate();
}

void Window::move(const Rectangle &area) {
  Root::move(area);
}

auto Window::content() const -> Rectangle {
  const auto size{area().size()};
  const auto inset{decoration().inset()};
  return Rectangle{
    inset,
    inset,
    std::max(inset, toDim(size.x() - inset)),
    std::max(inset, toDim(size.y() - inset))
  };
}

auto Window::decoration() const -> const Decoration & {
  return decoration_? *decoration_: defaultDecoration();
}

auto Window::colors() const -> const ColorStyle & {
  return colors_? *colors_: defaultColorStyle();
}

void Window::decorate() {
  write(Vector{0, 0}, decoration().draw(area().size(), state_, title_, background(), colors()));
}

void Window::title(const Text &title) {
  title_ = title;
  decorate();
}

void Window::decoration(const Decoration *decoration) {
  decoration_ = decoration;
  fill(background()); // nothing of the frame of the previous decoration may remain
  decorate();
  layout();
}

void Window::colors(const ColorStyle *colors) {
  colors_ = colors;
  decorate();
}

void Window::state(DecorationState state) {
  if (state != state_) {
    state_ = state;
    decorate();
  }
}

void Window::handleGeometry(const Rectangle &previous) {
  if (previous.size() != area().size()) {
    fill(background()); // the frame has to be drawn again
    decorate();
  }
  Root::handleGeometry(previous);
}

void Window::handleFocus() {
  state(DecorationState::active);
  Root::handleFocus();
}

void Window::handleBlur() {
  state(DecorationState::inactive);
  Root::handleBlur();
}

} // namespace jwezel::ui
