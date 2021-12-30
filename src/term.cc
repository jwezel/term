#include "term.hh"
#include "display.hh"
#include "fmt/core.h"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"
#include "text.hh"

using namespace jwezel;

Terminal::Terminal(
  const Char background,
  const Vector &initialPosition,
  const Vector &initialSize,
  const Vector &maxSize,
  int terminalFd,
  int keyboardFd,
  bool expand,
  bool contract
):
keyboard{keyboardFd == -1? terminalFd: keyboardFd},
display{keyboard, terminalFd, initialPosition, initialSize == VectorMin? Vector{1, 1}: initialSize, maxSize},
minimumSize(display.size()),
expand_(expand),
contract_(contract)
{
  auto [w, updates] = screen.addWindow(Rectangle{{0, 0}, display.size()}, background);
  desktop = w;
  display.update(updates);
}

jwezel::Window Terminal::newWindow(const Rectangle &area, const Char &background, const optional<jwezel::Window> &below) {
  expand(Vector{area.x2, area.y2});
  const auto [window, updates] = screen.addWindow(area, background, below.has_value()? below.value().window: 0);
  display.update(updates);
  return {*this, window};
}

void Terminal::deleteWindow(const jwezel::Window &window) {
  Vector size{window.window->area().x1, window.window->area().y1};
  const auto updates{screen.deleteWindow(window.window)};
  display.update(updates);
  contract();
}

void Terminal::moveWindow(const jwezel::Window &window, const Rectangle &area) {
  expand(Vector{area.x2, area.y2});
  display.update(screen.reshapeWindow(window.window, area));
  contract();
}

Rectangle Terminal::windowArea(const jwezel::Window &window) const {
  return window.window->area();
}

void Terminal::write(const jwezel::Window &window, const Vector &position, const Text &text) {
  display.update(screen.text(window.window, position, text));
}

void Terminal::focus(const jwezel::Window &window, bool status) {
  if (status) {
    screen.focus(window.window);
  } else {
    screen.unfocus(window.window);
  }
}

void Terminal::line(const jwezel::Window &window, const Line &line) {
  display.update(screen.line(window.window, line));
}

void Terminal::fill(const jwezel::Window &window, const Char &fill, const Rectangle &area) {
  display.update(screen.fill(window.window, fill, area));
}

void Terminal::box(const jwezel::Window &window, const Box &box) {
  display.update(screen.box(window.window, box));
}

bool Terminal::expand(const Vector &size) {
  if (!expand_) {
    return false;
  }
  const auto size_ = max(min(display.maxSize_, size), display.size());
  if (size_ != display.size()) {
    display.resize(size_);
    display.update(screen.reshapeWindow(desktop, Rectangle{Vector{0, 0}, size_}));
    return true;
  }
  return false;
}

bool Terminal::contract() {
  if (!contract_) {
    return false;
  }
  const auto size_{max(max(screen.minSize(desktop), {1, 1}), minimumSize)};
  if (size_ != display.size()) {
    display.update(screen.reshapeWindow(desktop, Rectangle{Vector{0, 0}, size_}));
    display.resize(size_);
    return true;
  }
  return false;
}

// ~Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void jwezel::Window::destroy() {
  terminal.deleteWindow(*this);
}

void jwezel::Window::move(const Rectangle &area) {
  terminal.moveWindow(*this, area);
}

Rectangle jwezel::Window::area() const {
  return terminal.windowArea(*this);
}

Vector jwezel::Window::size() const {
  return terminal.windowArea(*this).size();
}

Dim jwezel::Window::width() const {
  return terminal.windowArea(*this).width();
}

Dim jwezel::Window::height() const {
  return terminal.windowArea(*this).height();
}

void jwezel::Window::focus(bool status) {
  terminal.focus(*this, status);
}

void jwezel::Window::write(const Vector &position, const string_view &str) {
  terminal.write(*this, position, str);
}

void jwezel::Window::write(const Vector &position, const Text &text) {
  terminal.write(*this, position, text);
}

void jwezel::Window::box(const Box &box) {
  terminal.box(*this, box);
}

void jwezel::Window::line(const Line &line) {
  terminal.line(*this, line);
}

void jwezel::Window::fill(const Char &fill, const Rectangle &area) {
  terminal.fill(*this, fill, area);
}
