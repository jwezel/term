#include "term.hh"
#include "display.hh"
#include "event.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "text.hh"
#include "window.hh"

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
backdrop_{this},
screen_{&display_, {&backdrop_}},
desktop_{this, Rectangle{Vector{0, 0}, display_.size()}, background},
focusWindow_{&desktop_},
minimumSize_{display_.size()},
expand_{expand},
contract_{contract},
running_{false}
{}

void Terminal::registerWindow(Window *window) {
  focusWindow_ = window;
  expand(Vector{window->area().x2(), window->area().y2()});
}

void Terminal::moveWindow(Window &window, const Rectangle &area) {
  expand(Vector{area.x2(), area.y2()});
  screen_.reshapeElement(&window, area);
  contract();
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
    screen_.reshapeElement(&desktop_, Rectangle{Vector{0, 0}, size_});
    return true;
  }
  return false;
}

auto Terminal::contract() -> bool {
  if (!contract_) {
    return false;
  }
  const auto size_{max(max(screen_.minSize(&desktop_), Vector{1, 1}), minimumSize_)};
  if (size_ != display_.size()) {
    screen_.reshapeElement(&desktop_, Rectangle{Vector{0, 0}, size_});
    display_.resize(size_);
    return true;
  }
  return false;
}

} // namespace jwezel
