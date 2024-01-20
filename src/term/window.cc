#include "geometry.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <format>
#include <iostream>

namespace jwezel {

using std::format;

using std::cerr;

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

Window::Window(TerminalInterface *terminal, const Rectangle &area, const Char &background, Window *below):
TextElement(terminal->surface(), area, background, below),
terminal_{terminal}
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

auto Window::event(const Event &/*event*/) -> bool {
  return false;
}

}  // namespace jwezel
