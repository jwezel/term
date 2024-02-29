#include "container.hh"
#include "ui/window.hh"

#include <iostream>
#include <taitank.h>
#include <term/keyboard.hh>

namespace jwezel::ui {

Element::Element(struct Container *parent):
parent_(parent),
node_{taitank::NodeCreate()}
{
  if (parent) {
    dynamic_cast<Container *>(parent)->add(this);
  }
}

Element::~Element() {
  taitank::NodeFree(node_);
}

auto Element::window() /*NOLINT(misc-no-recursion)*/ -> Window * {
  auto *result{parent_? parent_->window(): 0};
  if (!result) {
    throw std::runtime_error("Can't find window");
  }
  return result;
}

auto Element::event(const Event &event) -> bool {
  switch (event.type()) {

    case MouseMoveEvent::type_:
    return mouseMoveEvent(
      Event{new MouseMoveEvent{dynamic_cast<MouseMoveEvent *>(event())->position() + window()->area().position()}}
    );
    break;

    case MouseButtonEvent::type_: {
      auto *buttonEvent{dynamic_cast<MouseButtonEvent *>(event())};
      return mouseButtonEvent(
        Event{
          new MouseButtonEvent{
            buttonEvent->button(),
            buttonEvent->modifiers(),
            buttonEvent->position() + window()->area().position(),
            buttonEvent->action()
          }
        }
      );
    }
    break;

    case KeyEvent::type_:
    return keyEvent(event);
    break;

    default:
    std::cerr << "Unhandled " << event.typeName() << " event\n";
    break;
  }
  return false;
}

auto Element::mouseMoveEvent(const Event &event) -> bool {
  for (auto &handler : eventHandlers_) {
    if (handler.first == MouseMoveEvent::type_) {
      return handler.second(*this, event);
    }
  }
  return false;
}

auto Element::mouseButtonEvent(const Event &event) -> bool {
  for (auto &handler : eventHandlers_) {
    if (handler.first == MouseButtonEvent::type_) {
      return handler.second(*this, event);
    }
  }
  return false;
}

auto Element::keyEvent(const Event &event) -> bool {
  for (auto &handler : eventHandlers_) {
    if (handler.first == KeyEvent::type_) {
      return handler.second(*this, event);
    }
  }
  return false;
}

auto Element::onMouseMove(const std::function<bool(Element &element, const Event &)>& handler) -> int {
  eventHandlers_.emplace_back(MouseMoveEvent::type_, handler);
  return static_cast<int>(eventHandlers_.size() - 1);
}

auto Element::onMouseButton(const std::function<bool(Element &element, const Event &)>& handler) -> int {
  eventHandlers_.emplace_back(MouseButtonEvent::type_, handler);
  return static_cast<int>(eventHandlers_.size() - 1);
}

auto Element::onKey(const std::function<bool(Element &element, const Event &)>& handler) -> int {
  eventHandlers_.emplace_back(KeyEvent::type_, handler);
  return static_cast<int>(eventHandlers_.size() - 1);
}


}  // namespace jwezel::ui
