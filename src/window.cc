#include "geometry.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <format>
#include <iostream>

namespace jwezel {

using std::format;

using std::cerr;

BaseWindow::BaseWindow(TerminalInterface *terminal, const Rectangle &area, const Char &background):
Surface::Element(terminal->screen().ptr(), area),
terminal_{terminal},
position_{area.position()},
background_{background}
{}

void BaseWindow::position(const Rectangle &area) {
  position_ = area.position();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(TerminalInterface *terminal, const Char &background):
BaseWindow(terminal, Rectangle{Vector{0, 0}, VectorMax}, background)
{}

Backdrop::~Backdrop() = default;

auto Backdrop::area() const -> Rectangle {
  return Rectangle{Vector{0, 0}, VectorMax};
}

auto Backdrop::text(const Rectangle &area) const -> Text {
  return Text{Space, area.size()};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Window::Window(TerminalInterface *terminal, const Rectangle &area, const Char &background, Window *below):
BaseWindow{terminal, area, background}
{
  text_.extend(area.size(), background);
  terminal->registerWindow(this);
  terminal->screen().addElement(this, below);
}

Window::~Window() {
  try {
    surface()->deleteElement(this);
    terminal()->contract();
  } catch (std::exception & error) {
    cerr << error.what() << '\n';
  }
}

Window::operator string() const {
  return format("Window(id={}, area={})", long(this), string(area()));
}

auto Window::write(const Vector &position, const string_view &str) -> Window & {
  return write(position, Text(str, RgbNone, RgbNone, {}, AttributeMode::mix));
}

auto Window::write(const Vector &position, const Text &txt_) -> Window & {
  text_.patch(txt_, position);
  update({Rectangle{position, position + txt_.size()}});
  return *this;
}

auto Window::fill(const Char &fillChar, const Rectangle &area) -> Window & {
  update({text_.fill(fillChar, area)});
  return *this;
}

auto Window::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Window & {
  update({text_.line(line, strength, dash, roundedCorners)});
  return *this;
}

auto Window::box(const Box &box) -> Window & {
  update(text_.box(box));
  return *this;
}

void Window::move(const Rectangle &area) {
  terminal()->moveWindow(*this, area);
  surface()->reshapeElement(this, area);
}

auto Window::moveEvent(const Rectangle &area) -> bool {
  position(area);
  text_.resize(area.size(), background());
  return true;
}

auto Window::above(Window *window) -> bool {
  surface()->above(this, window? window: surface()->zorder()[surface()->position(this) + 1]);
  return true;
}

auto Window::below(Window *window) -> bool {
  surface()->below(this, window? window: surface()->zorder()[surface()->position(this) - 1]);
  return true;
}

auto Window::above(int position) -> bool {
  surface()->above(this, surface()->zorder()[position < 0? position + surface()->zorder().size(): position]);
  return true;
}

auto Window::below(int position) -> bool {
  surface()->below(this, surface()->zorder()[position < 0? position + surface()->zorder().size(): position]);
  return true;
}

auto Window::area() const -> Rectangle {
  return Rectangle{position(), position() + text_.size()};
}

auto Window::text(const Rectangle &area) const -> Text {
  return text_[area];
}

}  // namespace jwezel
