#include "geometry.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <algorithm>
#include <format>

namespace jwezel {

using std::format;

//~Backdrop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(Surface *surface):
Element{surface}
{
  surface->addElement(this, 0);
}

auto Backdrop::area() const -> Rectangle {
  return Rectangle{Vector{0, 0}, VectorMax};
}

auto Backdrop::text(const Rectangle &area) const -> Text {
  return Text{Space, area.size()};
}

//~Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Window::Window(TerminalInterface *terminal, const Rectangle &area, const Char &background, Window *below, Window *transient):
ElementDevice(terminal->surface(), area, background, below),
terminal_{terminal},
transient_{transient}
{
  terminal->focus(this);
}

Window::~Window() = default;

Window::operator string() const {
  return format("Window(id={}, area={})", long(this), string(area()));
}

void Window::move(const Rectangle &area) {
  terminal()->moveWindow(*this, area);
  TextElement::move(area);
}

auto Window::clientInsets(const Insets &insets) -> void {
  if (insets == insets_) {
    return;
  }
  const auto previous{area()};
  insets_ = insets;
  geometryEvent(previous); // the content area changed; the UI root re-layouts
}

auto Window::clientArea() const -> Rectangle {
  const auto size{area().size()};
  const auto width{std::max(Dim{0}, toDim(size.x() - insets_.left - insets_.right))};
  const auto height{std::max(Dim{0}, toDim(size.y() - insets_.top - insets_.bottom))};
  return Rectangle{Vector{insets_.left, insets_.top}, Vector{toDim(insets_.left + width), toDim(insets_.top + height)}};
}

auto Window::event(const Event &event) -> bool {
  return inputHandler_? inputHandler_->handleEvent(event): false;
}

void Window::focusEvent() {
  if (inputHandler_) {
    inputHandler_->handleFocus();
  }
}

void Window::blurEvent() {
  if (inputHandler_) {
    inputHandler_->handleBlur();
  }
}

void Window::geometryEvent(const Rectangle &previous) {
  if (inputHandler_) {
    inputHandler_->handleGeometry(previous);
  }
}

}  // namespace jwezel
