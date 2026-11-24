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
  bool contract,
  Display::Screen screen
):
Surface{&display_},
expand_{expand},
contract_{contract},
keyboard_{keyboardFd == -1? terminalFd: keyboardFd},
display_{keyboard_, terminalFd, initialPosition, initialSize == VectorMin? Vector{1, 1}: initialSize, maxSize, screen},
backdrop_{this},
desktop_{this, Rectangle{Vector{0, 0}, display_.size()}, background},
minimumSize_{display_.size()},
running_{false}
{}

Terminal::~Terminal() {
  contract_ = false; // the display is going away; do not resize the desktop while it is being removed
}

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
    const auto *fragment{find(*caretPosition_)};
    visible = fragment and fragment->element->shows(*caretPosition_, caretOwner_);
  }
  display_.caret(visible? caretPosition_: std::nullopt);
}

void Terminal::focus(Window *window) {
  if (window and window->transient()) {
    window = window->transient(); // a transient cannot have the terminal focus; its owner has it
  }
  if (window == focusWindow_) {
    return;
  }
  auto *previous{focusWindow_};
  focusWindow_ = window;
  caretOwner_ = nullptr; // the caret of the previously focused window is gone
  caretPosition_.reset();
  placeCaret();
  if (previous) {
    previous->blurEvent();
  }
  if (window) {
    window->focusEvent();
  }
}

auto Terminal::windowAt(const Vector &position) -> Window * {
  if (auto *fragment{find(position)}; fragment) {
    return fragment->element->inputWindow();
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
    } else if (captureWindow_ and window != captureWindow_ and (not window or window->transient() != captureWindow_)) {
      // While a button is down, the motion and the release stay with the window that got the press,
      // except when the mouse is over a transient of it, like an open popup
      window = captureWindow_;
    }
    if (window and (not focusWindow_->modal() or window == focusWindow_ or window->transient() == focusWindow_)) {
      // A modal window takes the input exclusively while it is open and focused; no other window gets input,
      // but a transient of the modal window does: it belongs to it
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

void Terminal::start() {
  if (not focusWindow_) {
    focus(dynamic_cast<Window *>(zorder().back()));
  }
  running_ = true;
}

void Terminal::run() {
  start();
  while (running_) {
    runEvent();
  }
}

void Terminal::stop() {
  running_ = false;
}

auto Terminal::expand(const Vector &size) -> bool {
  if (not expand_) {
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
  if (not contract_) {
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
