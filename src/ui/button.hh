#pragma once

#include "label.hh"
#include "ui_event.hh"

#include <functional>
#include <vector>

namespace jwezel::ui {

/// A button was pressed; the cause is the key or mouse-button release that activated it.
struct PressEvent: UiEvent {
  using UiEvent::UiEvent;

  CLASS_ID(PressEvent);
};

/// A focusable label activated with Enter, Space, or a left mouse click.
///
/// The button is framed with the glyphs for its current UI state. A mouse press activates only if released inside
/// the button. Registered `Action`s let the button and a keyboard shortcut share behavior.
struct Button: Label {

  ///
  /// Create Button with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The text to display
  /// @param      background  The background
  Button(Container &parent, const Rectangle &area, const Text &text = {}, const Char &background = InheritBackground);

  ///
  /// Create Button with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The text to display
  /// @param      background  The background
  explicit Button(Container &parent, const Text &text = {}, const Char &background = InheritBackground);

  Button(const Button &) = delete;

  Button(Button &&) = delete;

  auto operator=(const Button &) -> Button & = delete;

  auto operator=(Button &&) -> Button & = delete;

  ~Button() override = default;

  ///
  /// Register a press handler
  ///
  /// @param[in]  handler  The handler, which is given a `PressEvent`: `event.as<PressEvent>()`
  ///
  /// @return     Index of the handler
  auto onPress(const Handler &handler) -> int;

  ///
  /// The size of the text between the brackets of the normal state
  [[nodiscard]] auto preferredSize() -> Vector override;

  protected:
  void render() override;

  private:
  void registerHandlers();

  void press(const Event &event);

  bool armed_{false}; // the left mouse button went down inside the button and is still down
};

} // namespace jwezel::ui
