#include "ui.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <algorithm>

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
  const ColorStyle *colors,
  const UiStyle *style
):
jwezel::Window{&ui.terminal(), DefaultSize(ui, area), background},
Root{static_cast<jwezel::Window &>(*this), colors, style},
framing_{}
{
  decoration_ = decoration;
  framing_.title = title;
  framing_.state = ui.terminal().focus() == static_cast<jwezel::Window *>(this)? DecorationState::active: DecorationState::inactive;
  decorate();
  layout(); // the client device takes the content area of the frame
}

void Window::move(const Rectangle &area) {
  Root::move(area);
}

auto Window::content() const -> Rectangle {
  const auto insets{framing_.insets()};
  const auto size{area().size()};
  const auto width{std::max(Dim{0}, toDim(size.x() - insets.left - insets.right))};
  const auto height{std::max(Dim{0}, toDim(size.y() - insets.top - insets.bottom))};
  return Rectangle{Vector{0, 0}, Vector{width, height}};
}

auto Window::contentOrigin() const -> Vector {
  const auto insets{framing_.insets()};
  return Vector{insets.left, insets.top};
}

auto Window::decoration() const -> const Decoration & {
  if (decoration_) {
    return *decoration_;
  }
  const auto &style{Root::style()};
  return style.decoration? *style.decoration: defaultDecoration();
}

void Window::decoration(const Decoration *decoration) {
  decoration_ = decoration;
  fill(background()); // nothing of the frame of the previous decoration may remain
  decorate();
  layout();
}

void Window::decorate() {
  framing_.decoration = &decoration(); // the decoration is re-resolved here so that a style change shows
  framing_.colors = &Root::colors();
  write(Vector{0, 0}, framing_.draw(area().size(), background()));
}

void Window::title(const Text &title) {
  framing_.title = title;
  decorate();
}

void Window::restyle() {
  fill(background()); // the decoration may have changed
  decorate();
  layout(); // and with it the room for the content
  Root::restyle();
}

void Window::state(DecorationState state) {
  if (state != framing_.state) {
    framing_.state = state;
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
