#include <fmt/core.h>

#include "_screen_window.hh"
#include "geometry.hh"
#include "text.hh"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace jwezel::screen {

using fmt::format;
using std::runtime_error;

#define NOT_IMPLEMENTED throw runtime_error(format("Not implemented in {}", typeid(this).name()));

BaseWindow::BaseWindow(const Rectangle &area, const Char &background):
Surface::Element{area},
position{area.position()},
background{background}
{}

BaseWindow::operator string() const {
  return format("{}}(id={}, area={})", typeid(this).name(), long(this), string(area()));
}

void BaseWindow::write(const Vector & /*unused*/, const string_view & /*unused*/) {
  NOT_IMPLEMENTED
}

void BaseWindow::write(const Vector & /*unused*/, const Text & /*unused*/) {
  NOT_IMPLEMENTED
}

void BaseWindow::fill(const Char & /*unused*/, const Rectangle & /*unused*/) {
  NOT_IMPLEMENTED
}

auto BaseWindow::line(const Line & /*unused*/, u1 /*unused*/, u1 /*unused*/, bool /*unused*/) -> Rectangle {
  NOT_IMPLEMENTED
}

auto BaseWindow::box(const Box & /*unused*/) -> vector<Rectangle> {
  NOT_IMPLEMENTED
}

void BaseWindow::move(const Rectangle & /*area*/) {
  NOT_IMPLEMENTED
}

auto BaseWindow::area() const -> Rectangle {
  NOT_IMPLEMENTED
}

auto BaseWindow::size() const -> Vector {
  NOT_IMPLEMENTED
}

auto BaseWindow::text() const -> const Text & {
  NOT_IMPLEMENTED
}

auto BaseWindow::text(const Rectangle & /*area*/) const -> Text {
  NOT_IMPLEMENTED
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(const Char &background):
BaseWindow{Rectangle{Vector{0, 0}, VectorMax}, background}
{}

auto Backdrop::area() const -> Rectangle {
  return Rectangle{Vector{0, 0}, VectorMax};
}

auto Backdrop::size() const -> Vector {
  return VectorMax;
}

auto Backdrop::text() const -> const Text & {
  NOT_IMPLEMENTED
}

auto Backdrop::text(const Rectangle &area) const -> Text {
  return Text{Space, area.size()};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Window::Window(const Rectangle &area, const Char &background):
BaseWindow{area, background}
{
  text_.extend(area.size(), background);
}

Window::operator string() const {
  return format("Window(id={}, area={})", long(this), string(area()));
}

void Window::write(const Vector &position, const string_view &str) {
  text_.patch(Text(str, RgbNone, RgbNone, {}, mix), position);
}

void Window::write(const Vector &position, const Text &txt_) {
  text_.patch(txt_, position);
}

void Window::fill(const Char &fillChar, const Rectangle &area) {
  text_.fill(fillChar, area);
}

auto Window::line(
  const Line &line,
  u1 strength,
  u1 dash,
  bool roundedCorners
) -> Rectangle {
  return text_.line(line, strength, dash, roundedCorners);
}

auto Window::box(const Box &box) -> vector<Rectangle> {
  return text_.box(box);
}

void Window::move(const Rectangle &area) {
  position = area.position();
  text_.resize(area.size(), background);
}

auto Window::size() const -> Vector {
  return text_.size();
}

auto Window::area() const -> Rectangle {
  return Rectangle{position, position + text_.size()};
}

} // namespace jwezel::screen
