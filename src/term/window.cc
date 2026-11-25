#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"

#include <algorithm>
#include <format>

namespace jwezel {

using std::format;

//~Backdrop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(Surface *surface):
Element{surface}
{
  surface->addElement(this, 0);
}

auto Backdrop::area() const -> Rectangle {
  return Rectangle{Vector{0, 0}, VectorMax};
}

auto Backdrop::text(const Rectangle &area) const -> Text {
  return Text{Space, area.size()};
}

//~Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Window::Window(TerminalInterface *terminal, const Rectangle &area, const Char &background, Window *below, Window *transient):
ElementDevice(terminal->surface(), area, background, below),
terminal_{terminal},
transient_{transient}
{
  terminal->focus(this);
}

Window::~Window() = default;

Window::operator string() const {
  return format("Window(id={}, area={})", long(this), string(area()));
}

void Window::move(const Rectangle &area) {
  terminal()->moveWindow(*this, area);
  TextElement::move(area);
}

auto Window::clientInsets(const Insets &insets) -> void {
  if (insets == insets_) {
    return;
  }
  const auto previous{area()};
  insets_ = insets;
  geometryEvent(previous); // the content area changed; the UI root re-layouts
}

auto Window::clientArea() const -> Rectangle {
  const auto size{area().size()};
  const auto width{std::max(Dim{0}, toDim(size.x() - insets_.left - insets_.right))};
  const auto height{std::max(Dim{0}, toDim(size.y() - insets_.top - insets_.bottom))};
  return Rectangle{Vector{insets_.left, insets_.top}, Vector{toDim(insets_.left + width), toDim(insets_.top + height)}};
}

auto Window::event(const Event &event) -> bool {
  // The input handler sees the event first, so a click on the frame still focuses the window and closes popups
  auto handled{inputHandler_? inputHandler_->handleEvent(event): false};
  if (event.type() == MouseButtonEvent::type_) {
    const auto &mouse{event.as<MouseButtonEvent>()};
    if (mouse.button() == MouseButton::Button1 and not mouse.modifiers().mod5) {
      if (mouse.action() == MouseAction::Down) {
        moving_ = false;
        resizeX_ = 0;
        resizeY_ = 0;
        if (beginGesture(mouse.position())) {
          return true;
        }
      } else if (moving_ or resizeX_ != 0 or resizeY_ != 0) {
        moving_ = false;
        resizeX_ = 0;
        resizeY_ = 0;
        return true;
      }
    }
  } else if (event.type() == MouseMoveEvent::type_ and (moving_ or resizeX_ != 0 or resizeY_ != 0)) {
    return continueGesture(event.as<MouseMoveEvent>().position());
  }
  return handled;
}

auto Window::beginGesture(const Vector &position) -> bool {
  const auto size{area().size()};
  const auto local{position - area().position()};
  const auto content{clientArea()};
  if (local.x() < 0 or local.y() < 0 or local.x() >= size.x() or local.y() >= size.y()) {
    return false;
  }
  if (
    local.x() >= content.x1() and local.x() < content.x2()
    and local.y() >= content.y1() and local.y() < content.y2()
  ) {
    return false; // inside the client area: not a window management gesture
  }
  gestureArea_ = area();
  gesturePress_ = position;
  if (local.y() < content.y1()) {
    moving_ = true; // the title bar
    return true;
  }
  resizeX_ = local.x() < content.x1()? -1: local.x() >= content.x2()? 1: 0;
  resizeY_ = local.y() >= content.y2()? 1: 0;
  if (resizeX_ == 0 and resizeY_ == 0) {
    moving_ = true; // another part of the frame, like a thick title band
  }
  return true;
}

auto Window::continueGesture(const Vector &position) -> bool {
  const auto delta{position - gesturePress_};
  const auto display{terminal_->maxSize()};
  const auto minimumWidth{toDim(insets_.left + insets_.right + 1)};
  const auto minimumHeight{toDim(insets_.top + insets_.bottom + 1)};
  auto x1{gestureArea_.x1()}, y1{gestureArea_.y1()}, x2{gestureArea_.x2()}, y2{gestureArea_.y2()};
  if (moving_) {
    const auto width{gestureArea_.width()}, height{gestureArea_.height()};
    const auto maxX{std::max(Dim{0}, toDim(display.x() - width))};
    const auto maxY{std::max(Dim{0}, toDim(display.y() - height))};
    x1 = std::clamp(toDim(gestureArea_.x1() + delta.x()), Dim{0}, maxX);
    y1 = std::clamp(toDim(gestureArea_.y1() + delta.y()), Dim{0}, maxY);
    x2 = toDim(x1 + width);
    y2 = toDim(y1 + height);
  } else {
    if (resizeX_ < 0) {
      const auto upper{std::min(toDim(x2 - minimumWidth), display.x())};
      x1 = std::clamp(toDim(x1 + delta.x()), Dim{0}, std::max(Dim{0}, upper));
    } else if (resizeX_ > 0) {
      const auto lower{std::min(toDim(x1 + minimumWidth), display.x())};
      x2 = std::clamp(toDim(x2 + delta.x()), lower, std::max(lower, display.x()));
    }
    if (resizeY_ > 0) {
      const auto lower{std::min(toDim(y1 + minimumHeight), display.y())};
      y2 = std::clamp(toDim(y2 + delta.y()), lower, std::max(lower, display.y()));
    }
  }
  const Rectangle next{x1, y1, x2, y2};
  if (next != area()) {
    move(next);
  }
  return true;
}

void Window::focusEvent() {
  if (inputHandler_) {
    inputHandler_->handleFocus();
  }
}

void Window::blurEvent() {
  if (inputHandler_) {
    inputHandler_->handleBlur();
  }
}

void Window::geometryEvent(const Rectangle &previous) {
  if (inputHandler_) {
    inputHandler_->handleGeometry(previous);
  }
}

}  // namespace jwezel
