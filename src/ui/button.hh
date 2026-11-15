#pragma once

#include "label.hh"

#include <functional>
#include <vector>

namespace jwezel::ui {

///
/// Button widget
///
/// Displays its text framed by brackets and can be pressed with the Space
/// key or with a left mouse button click inside its area. Press handlers
/// registered with onPress() are invoked with the triggering event.
struct Button: Label {
  using Handler = std::function<bool(const ui::Element &element, const Event &event)>;

  ///
  /// Create Button with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The text to display
  /// @param      background  The background
  explicit Button(Container &parent, const Rectangle &area, const Text &text = {}, const Char &background = Space);

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
  /// @param[in]  handler  The handler
  ///
  /// @return     Index of the handler
  auto onPress(const Handler &handler) -> int;

  protected:
  void render() override;

  private:
  void registerHandlers();

  void press(const Event &event);

  std::vector<Handler> pressHandlers_;
};

} // namespace jwezel::ui
