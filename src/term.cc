#include "term.hh"
#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"

using namespace jwezel;

Terminal::Terminal(
  const Vector &initialPosition,
  const Vector &initialSize,
  const Vector &maxSize,
  int terminalFd,
  int keyboardFd
):
keyboard(keyboardFd == -1? terminalFd: keyboardFd),
display(keyboard, terminalFd, initialPosition, initialSize, maxSize),
screen(Rectangle{display.position_, display.size()})
{}

Terminal::WinId Terminal::newWindow(const Rectangle &area, WinId below) {
  expand(area);
  auto [window, updates] = screen.addWindow(area, below < 0? 0: screen[below]);
  display.update(updates);
  return screen[window];
}
