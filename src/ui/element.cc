#include "container.hh"
#include "ui/window.hh"

#include <iostream>
#include <term/keyboard.hh>
#include <taitank.h>

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
    return onMouseMove(
      Event{new MouseMoveEvent{dynamic_cast<MouseMoveEvent *>(event())->position() + window()->area().position()}}
    );
    break;

    case MouseButtonEvent::type_:
    return onMouseButton(
      Event{new MouseMoveEvent{dynamic_cast<MouseMoveEvent *>(event())->position() + window()->area().position()}}
    );
    break;

    case KeyEvent::type_:
    return onKey(event);
    break;

    default:
    std::cerr << "Unhandled " << event.typeName() << " event\n";
    break;
  }
  return false;
}

}  // namespace jwezel::ui
