#include "term.hh"
#include "display.hh"
#include "event.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <unistd.h>

namespace jwezel {

Terminal::Terminal(
  const Char &background,
  const Vector &initialPosition,
  const Vector &initialSize,
  const Vector &maxSize,
  int terminalFd,
  int keyboardFd,
  bool expand,
  bool contract
):
Surface{&display_},
expand_{expand},
contract_{contract},
keyboard_{keyboardFd == -1? terminalFd: keyboardFd},
display_{keyboard_, terminalFd, initialPosition, initialSize == VectorMin? Vector{1, 1}: initialSize, maxSize},
backdrop_{this},
desktop_{this, Rectangle{Vector{0, 0}, display_.size()}, background},
focusWindow_{&desktop_},
minimumSize_{display_.size()},
running_{false}
{}

void Terminal::addElement(Surface::Element *element, Surface::Element * below) {
  if (element != &backdrop_) {
    expand(element->area().position2());
  }
  Surface::addElement(element, below);
}

void Terminal::deleteElement(Element *element, Element *destination) {
  Surface::deleteElement(element, destination);
  contract();
}

void Terminal::moveWindow(Window &window, const Rectangle &area) {
  expand(Vector{area.x2(), area.y2()});
  reshapeElement(&window, area);
  contract();
}

void Terminal::focus(Window *window) {
  focusWindow_ = window;
}

auto Terminal::event() -> Event * {
  return keyboard_.event();
}

void Terminal::run() {
  while (running_) {
    (void)keyboard_.event();
  }
}

auto Terminal::expand(const Vector &size) -> bool {
  if (!expand_) {
    return false;
  }
  const auto size_ = max(min(display_.maxSize(), size), display_.size());
  if (size_ != display_.size()) {
    display_.resize(size_);
    reshapeElement(&desktop_, Rectangle{Vector{0, 0}, size_});
    return true;
  }
  return false;
}

auto Terminal::contract() -> bool {
  if (!contract_) {
    return false;
  }
  const auto size_{max(max(minSize(&desktop_), Vector{1, 1}), minimumSize_)};
  if (size_ != display_.size()) {
    reshapeElement(&desktop_, Rectangle{Vector{0, 0}, size_});
    display_.resize(size_);
    return true;
  }
  return false;
}

} // namespace jwezel
