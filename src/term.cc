#include "term.hh"
#include "display.hh"
#include "event.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"
#include "text.hh"
#include <algorithm>
#include <unistd.h>

namespace jwezel {

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
keyboard_{keyboardFd == -1? terminalFd: keyboardFd},
display_{keyboard_, terminalFd, initialPosition, initialSize == VectorMin? Vector{1, 1}: initialSize, maxSize},
minimumSize_(display_.size()),
expand_(expand),
contract_(contract),
running_{false}
{
  auto [window, updates] = screen_.addWindow(Rectangle{Vector{0, 0}, display_.size()}, background);
  desktop_ = window; // NOLINT (needs elaborate initialization)
  display_.update(updates);
}

auto Terminal::newWindow(const Rectangle &area, const Char &background, const optional<jwezel::Window> &below) -> jwezel::Window {
  expand(Vector{area.x2(), area.y2()});
  const auto [window_, updates]{screen_.addWindow(area, background, below.has_value()? below.value().window(): 0)};
  display_.update(updates);
  return {*this, window_};
}

void Terminal::deleteWindow(const jwezel::Window &window) {
  Vector size{window.area().x1(), window.area().y1()};
  const auto updates{screen_.deleteWindow(window.window())};
  display_.update(updates);
  contract();
}

void Terminal::moveWindow(const jwezel::Window &window, const Rectangle &area) {
  expand(Vector{area.x2(), area.y2()});
  display_.update(screen_.reshapeWindow(window.window(), area));
  contract();
}

void Terminal::write(const jwezel::Window &window, const Vector &position, const Text &text) {
  display_.update(screen_.text(window.window(), position, text));
}

void Terminal::focus(const jwezel::Window &window, bool status) {
  if (status) {
    screen_.focus(window.window());
  } else {
    screen_.unfocus(window.window());
  }
}

void Terminal::line(const jwezel::Window &window, const Line &line) {
  display_.update(screen_.line(window.window(), line));
}

void Terminal::fill(const jwezel::Window &window, const Char &fill, const Rectangle &area) {
  display_.update(screen_.fill(window.window(), fill, area));
}

void Terminal::box(const jwezel::Window &window, const Box &box) {
  display_.update(screen_.box(window.window(), box));
}

auto Terminal::event() -> Event * {
  return keyboard_.event();
}

void Terminal::run() {
  while (running_) {
    keyboard_.event();
  }
}

auto Terminal::expand(const Vector &size) -> bool {
  if (!expand_) {
    return false;
  }
  const auto size_ = max(min(display_.maxSize_, size), display_.size());
  if (size_ != display_.size()) {
    display_.resize(size_);
    display_.update(screen_.reshapeWindow(desktop_, Rectangle{Vector{0, 0}, size_}));
    return true;
  }
  return false;
}

auto Terminal::contract() -> bool {
  if (!contract_) {
    return false;
  }
  const auto size_{max(max(screen_.minSize(desktop_), Vector{1, 1}), minimumSize_)};
  if (size_ != display_.size()) {
    display_.update(screen_.reshapeWindow(desktop_, Rectangle{Vector{0, 0}, size_}));
    display_.resize(size_);
    return true;
  }
  return false;
}

// ~Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void jwezel::Window::destroy() {
  terminal_.deleteWindow(*this);
}

void jwezel::Window::move(const Rectangle &area) {
  terminal_.moveWindow(*this, area);
}

auto jwezel::Window::area() const -> Rectangle {
  return window_->area();
}

auto jwezel::Window::size() const -> Vector {
  return window_->area().size();
}

auto jwezel::Window::width() const -> Dim {
  return window_->area().width();
}

auto jwezel::Window::height() const -> Dim {
  return window_->area().height();
}

void jwezel::Window::focus(bool status) {
  terminal_.focus(*this, status);
}

void jwezel::Window::write(const Vector &position, const string_view &str) {
  window_->write(position, Text(str, RgbNone, RgbNone, {}, mix));
}

void jwezel::Window::write(const Vector &position, const Text &text) {
  window_->write(position, text);
}

void jwezel::Window::box(const Box &box) {
  window_->box(box);
}

void jwezel::Window::line(const Line &line) {
  window_->line(line);
}

void jwezel::Window::fill(const Char &fill, const Rectangle &area) {
  window_->fill(fill, area);
}

} // namespace jwezel
