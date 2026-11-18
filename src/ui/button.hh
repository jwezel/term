#pragma once

#include "label.hh"
#include "ui_event.hh"

#include <functional>
#include <vector>

namespace jwezel::ui {

///
/// Button widget
///
/// Displays its text framed by brackets (those of the UI style for the state of the button: by default angle
/// brackets show that it has the focus) and can be pressed with the Space or
/// Enter key or with a left mouse button click: the button has to be released
/// inside the button as well as pressed, so a click can be called off by
/// moving the mouse out of the button before releasing it. Press handlers
/// registered with onPress() are invoked with the triggering event, and then the
/// actions that were added to the button are triggered: pressing it is how it
/// does what an action stands for. The colors
/// are those of the roles `button`, `button.focus` (by default a tint) and,
/// while the mouse button is down on it, `button.active`.
///
/// A button was pressed; the cause is the key or the release of the mouse button that did it
struct PressEvent: UiEvent {
  using UiEvent::UiEvent;

  CLASS_ID(PressEvent);
};

struct Button: Label {

  ///
  /// Create Button with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The text to display
  /// @param      background  The background
  Button(Container &parent, const Rectangle &area, const Text &text = {}, const Char &background = Space);

  ///
  /// Create Button with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The text to display
  /// @param      background  The background
  explicit Button(Container &parent, const Text &text = {}, const Char &background = Space);

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

  protected:
  void render() override;

  private:
  void registerHandlers();

  void press(const Event &event);

  bool armed_{false}; // the left mouse button went down inside the button and is still down
};

} // namespace jwezel::ui
