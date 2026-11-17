#include "element.hh"
#include "container.hh"
#include "root.hh"

#include <iostream>
#include <term/keyboard.hh>

namespace jwezel::ui {

Element::Element(struct Container *parent):
parent_(parent)
{}

Element::~Element() {
  if (parent_) {
    parent_->remove(*this);
  }
}

auto Element::root() /*NOLINT(misc-no-recursion)*/ -> Root * {
  auto *result{parent_? parent_->root(): 0};
  if (!result) {
    throw std::runtime_error("Can't find root");
  }
  return result;
}

auto Element::event(const Event &event) -> bool {
  switch (event.type()) {

    case MouseMoveEvent::type_:
    return mouseMoveEvent(
      Event{new MouseMoveEvent{event.as<MouseMoveEvent>().position() - root()->origin()}}
    );
    break;

    case MouseButtonEvent::type_: {
      const auto &buttonEvent{event.as<MouseButtonEvent>()};
      return mouseButtonEvent(
        Event{
          new MouseButtonEvent{
            buttonEvent.button(),
            buttonEvent.modifiers(),
            buttonEvent.position() - root()->origin(),
            buttonEvent.action()
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
    if (handler.first == MouseMoveEvent::type_ and handler.second(*this, event)) {
      return true;
    }
  }
  return false;
}

auto Element::mouseButtonEvent(const Event &event) -> bool {
  for (auto &handler : eventHandlers_) {
    if (handler.first == MouseButtonEvent::type_ and handler.second(*this, event)) {
      return true;
    }
  }
  return false;
}

auto Element::keyEvent(const Event &event) -> bool {
  for (auto &handler : eventHandlers_) {
    if (handler.first == KeyEvent::type_ and handler.second(*this, event)) {
      return true;
    }
  }
  return false;
}

auto Element::onMouseMove(const Handler &handler) -> int {
  return on<MouseMoveEvent>(handler);
}

auto Element::onMouseButton(const Handler &handler) -> int {
  return on<MouseButtonEvent>(handler);
}

auto Element::onKey(const Handler &handler) -> int {
  return on<KeyEvent>(handler);
}


}  // namespace jwezel::ui
