#include "geometry.hh"
#include "ui/ui.hh"
#include "ui/window.hh"

#include <fmt/core.h>

namespace jwezel::ui {

static auto DefaultSize(Ui &ui, const Rectangle &area) {
  if (area == RectangleDefault) {
    return Rectangle{Vector{0, 0}, ui.terminal().display().maxSize() * 3 / 2};
  }
  return area;
}

Window::Window(Ui &ui, const Char &background, const Rectangle &area):
VisibleElement(background, 0),
window_{ui.terminal().newWindow(DefaultSize(ui, area), background)}
{
  ui.add(this);
}

auto Window::area() const -> Rectangle {
  return window_.area();
}

void Window::area(const Rectangle &area) {
  if (area != RectangleDefault) {
  }
}

auto Window::size() const -> Vector {
  return window_.size();
}

auto Window::width() const -> Dim {
  return window_.width();
}

auto Window::height() const -> Dim {
  return window_.height();
}

} // namespace jwezel::ui
