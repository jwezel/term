#include "term.hh"
#include "display.hh"
#include "fmt/core.h"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"

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

Terminal::WinId Terminal::newWindow(const Rectangle &area, const Char &background, WinId below) {
  expand(Vector{area.x2, area.y2});
  const auto [window, updates] = screen.addWindow(area, background, below < 0? 0: screen[below]);
  display.update(updates);
  return screen[window];
}

void Terminal::deleteWindow(WinId windowId) {
  const auto window{screen[windowId]};
  Vector size{window->area().x1, window->area().y1};
  const auto updates{screen.deleteWindow(window)};
  display.update(updates);
  contract();
}

void Terminal::moveWindow(WinId windowId, const Rectangle &area) {
  const auto window{screen[windowId]};
  expand(Vector{area.x2, area.y2});
  display.update(screen.reshapeWindow(window, area));
  contract();
}

void Terminal::write(WinId windowId, const Vector &position, const Text &text) {
  display.update(screen.text(screen[windowId], position, text));
}

void Terminal::box(WinId windowId, const Box &box) {
  display.update(screen.box(screen[windowId], box));
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
