#include "term.hh"
#include "display.hh"
#include "event.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <stdexcept>
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
minimumSize_{display_.size()},
running_{false}
{}

void Terminal::addElement(Surface::Element *element, Surface::Element * below) {
  if (element != &backdrop_) {
    expand(element->area().position2());
  }
  Surface::addElement(element, below);
  placeCaret();
}

void Terminal::deleteElement(Element *element, Element *destination) {
  if (element == caretOwner_) {
    caretOwner_ = nullptr;
    caretPosition_.reset();
  }
  if (element == focusWindow_) {
    focusWindow_ = element == &desktop_? nullptr: &desktop_; // the desktop takes over the focus
  }
  if (element == captureWindow_) {
    captureWindow_ = nullptr;
  }
  Surface::deleteElement(element, destination);
  contract();
  placeCaret();
}

void Terminal::reshapeElement(Element *element, const Rectangle &area) {
  expand(area.position2()); // the display grows to fit, whichever element it is
  const auto previous{element->area()};
  Surface::reshapeElement(element, area);
  if (auto *window{element->inputWindow()}; window == element and previous != area) {
    window->geometryEvent(previous); // windows have to follow their geometry, e.g. to lay out their UI
  }
  placeCaret();
}

void Terminal::moveWindow(Window &window, const Rectangle &area) {
  expand(Vector{area.x2(), area.y2()});
  reshapeElement(&window, area);
  contract();
}

void Terminal::caret(Window &window, const Surface::Element *owner, const std::optional<Vector> &position) {
  if (&window == focusWindow_) {
    caretOwner_ = owner;
    caretPosition_ = position;
    placeCaret();
  }
}

void Terminal::placeCaret() {
  bool visible{false};
  if (caretOwner_ and caretPosition_) {
    Fragment *fragment{};
    try {
      fragment = find(*caretPosition_);
    } catch (const std::logic_error &) { // no single topmost element at this position
      fragment = nullptr;
    }
    visible = fragment and fragment->element == caretOwner_;
  }
  display_.caret(visible? caretPosition_: std::nullopt);
}

void Terminal::focus(Window *window) {
  if (window == focusWindow_) {
    return;
  }
  focusWindow_ = window;
  caretOwner_ = nullptr; // the caret of the previously focused window is gone
  caretPosition_.reset();
  placeCaret();
  if (window) {
    window->focusEvent();
  }
}

auto Terminal::windowAt(const Vector &position) -> Window * {
  try {
    if (auto *fragment{find(position)}; fragment) {
      return fragment->element->inputWindow();
    }
  } catch (const std::logic_error &) { // no single topmost element at this position
  }
  return nullptr;
}

auto Terminal::event() -> Event {
  return keyboard_.event();
}

void Terminal::runEvent() {
  auto currentEvent{keyboard_.event()};
  auto *target{focusWindow_};
  auto change{MouseCapture::Change::none};
  // Mouse input goes to the window under the mouse, whether or not it has the focus;
  // it is up to the window to claim the focus. Once a button is down, the motion and
  // the release stay with the window that got the press.
  if (currentEvent.type() == MouseButtonEvent::type_ or currentEvent.type() == MouseMoveEvent::type_) {
    if (currentEvent.type() == MouseButtonEvent::type_) {
      change = mouseCapture_.track(*dynamic_cast<MouseButtonEvent *>(currentEvent()));
    }
    auto *window{
      captureWindow_ and change != MouseCapture::Change::began?
        captureWindow_
      :
        windowAt(dynamic_cast<MouseEvent *>(currentEvent())->position())
    };
    if (change == MouseCapture::Change::began) {
      captureWindow_ = window;
    }
    if (window) {
      target = window;
    }
  }
  if (target) {
    target->event(currentEvent);
  }
  if (change == MouseCapture::Change::ended) {
    captureWindow_ = nullptr;
  }
}

void Terminal::run() {
  if (!focusWindow_) {
    focus(dynamic_cast<Window *>(zorder().back()));
  }
  running_ = true;
  while (running_) {
    runEvent();
  }
}

void Terminal::stop() {
  running_ = false;
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
