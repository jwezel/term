#include <fmt/core.h>

#include <cmath>
#include <vector>
#include "window.hh"
#include "geometry.hh"

using namespace jwezel;

Window::Window(const Rectangle &area):
area{area},
fragments{area}
{
  fill();
}

Window::operator string() const {
  return fmt::format("Window(id={}, area={})", long(this), string(area));
}

void Window::write(const Vector &position, const string_view &str) {
  text.patch(Text(str), position);
}

void Window::write(const Vector &position, const Text &text_) {
  text.patch(text_, position);
}

void Window::fill(const Char &fillChar) {
  text.extend(Vector(area.width(), area.height()), fillChar);
  text.fill(fillChar);
}

Rectangle Window::line(
  const Line &line,
  u1 strength,
  u1 dash,
  bool roundedCorners
) {
  return text.line(line, strength, dash, roundedCorners);
}

vector<Rectangle> Window::box(const Box &box) {
  return text.box(box);
}
