#include <fmt/core.h>

#include <cmath>
#include <stdexcept>
#include <vector>
#include "_screen_window.hh"
#include "geometry.hh"
#include "text.hh"

using namespace jwezel;
using namespace jwezel::screen;
using fmt::format;

#define NOT_IMPLEMENTED throw runtime_error(format("Not implemented in {}", typeid(this).name()));

BaseWindow::BaseWindow(const Rectangle &area, const Char &background):
position{area.position()},
background{background},
fragments{area}
{}

BaseWindow::operator string() const {
  return format("{}}(id={}, area={})", typeid(this).name(), long(this), string(area()));
}

void BaseWindow::write(const Vector &, const string_view &) {
  NOT_IMPLEMENTED
}

void BaseWindow::write(const Vector &, const Text &) {
  NOT_IMPLEMENTED
}

void BaseWindow::fill(const Char &, const Rectangle &) {
  NOT_IMPLEMENTED
}

Rectangle BaseWindow::line(const Line &, u1, u1, bool) {
  NOT_IMPLEMENTED
}

vector<Rectangle> BaseWindow::box(const Box &) {
  NOT_IMPLEMENTED
}

void BaseWindow::move(const Rectangle &) {
  NOT_IMPLEMENTED
}

Rectangle BaseWindow::area() const {
  NOT_IMPLEMENTED
}

Vector BaseWindow::size() const {
  NOT_IMPLEMENTED
}

const Text &BaseWindow::text() const {
  NOT_IMPLEMENTED
}

Text BaseWindow::text(const Rectangle &) const {
  NOT_IMPLEMENTED
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(const Char &background):
BaseWindow{Rectangle{{0, 0}, VectorMax}, background}
{}

Rectangle Backdrop::area() const {
  return Rectangle{{0, 0}, VectorMax};
}

Vector Backdrop::size() const {
  return VectorMax;
}

const Text &Backdrop::text() const {
  NOT_IMPLEMENTED
}

Text Backdrop::text(const Rectangle &area) const {
  return Text(Space, area.size());
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

void Window::write(const Vector &position, const Text &text__) {
  text_.patch(text__, position);
}

void Window::fill(const Char &fillChar, const Rectangle &area) {
  text_.fill(fillChar, area);
}

Rectangle Window::line(
  const Line &line,
  u1 strength,
  u1 dash,
  bool roundedCorners
) {
  return text_.line(line, strength, dash, roundedCorners);
}

vector<Rectangle> Window::box(const Box &box) {
  return text_.box(box);
}

void Window::move(const Rectangle &area) {
  position = area.position();
  text_.resize(area.size(), background);
}

Vector Window::size() const {
  return text_.size();
}

Rectangle Window::area() const {
  return Rectangle{position, position + text_.size()};
}
