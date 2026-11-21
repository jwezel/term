#pragma once

#include <term/event.hh>
#include <term/keyboard.hh>

#include <stdexcept>
#include <utility>
#include <variant>

namespace jwezel::ui {

///
/// Something an element reports about itself, because of input
///
/// Widgets emit their own kinds of events (`PressEvent`, `SelectEvent`, `AcceptEvent`), which are all `UiEvent`s.
/// A handler is given the `Event` as it is for all handlers, and gets the kind it has been registered for with
/// `event.as<SelectEvent>()`.
///
/// The event has a copy of the input event that caused it, so it is as valid as the event is. A mouse event has
/// the position relative to the window, like the events elements are given. A list selection changed while dragging
/// carries the mouse-move event that changed that item.
struct UiEvent: BaseEvent {
  using Cause = std::variant<KeyEvent, MouseButtonEvent, MouseMoveEvent>;

  explicit UiEvent(Cause cause):
  cause_{std::move(cause)}
  {}

  ///
  /// Make the cause of an event from the input event that is handled
  ///
  /// @throws     std::logic_error if the input event is not a key or mouse event
  [[nodiscard]] static auto causeOf(const Event &input) -> Cause {
    if (const auto *key{input.get<KeyEvent>()}; key) {
      return *key;
    }
    if (const auto *mouse{input.get<MouseButtonEvent>()}; mouse) {
      return *mouse;
    }
    if (const auto *mouse{input.get<MouseMoveEvent>()}; mouse) {
      return *mouse;
    }
    throw std::logic_error(std::string{"An input event is not a key or mouse event: "} + input.typeName());
  }

  ///
  /// Get the input that caused the event, as what it is: `std::get_if<KeyEvent>(&event.cause())`
  [[nodiscard]] auto cause() const -> const Cause & {return cause_;}

  ///
  /// Get the input that caused the event, as any input event
  [[nodiscard]] auto input() const -> const InputEvent & {
    return std::visit([](const auto &input) -> const InputEvent & {return input;}, cause_);
  }

  private:
  Cause cause_;

  CLASS_ID(UiEvent);
};

} // namespace jwezel::ui
