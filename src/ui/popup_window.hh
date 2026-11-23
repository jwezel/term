#pragma once

#include "root.hh"

#include <term/geometry.hh>
#include <term/window.hh>

namespace jwezel::ui {

///
/// Borderless transient window that hosts the popup of a choice control
///
/// A popup that reaches past the window of its control is a window of its own, transient to the window of the
/// control: the owner keeps the terminal focus, and input that lands on the popup is routed to it while the owner has
/// the focus. Its root holds the popup widget, whose areas are relative to the popup window's content, which is the
/// whole window, because the popup has no frame. The popup is positioned by its owner so that it fits on the display;
/// a popup does not grow the display, so `ensure` does nothing.
///
/// The color style and the UI style are the ones of the root of the control, which the popup takes over, so that its
/// widgets look like the ones of the window it belongs to. Both are non-owning weak references that must outlive the
/// popup.
///
/// A popup window is closed while it handles input: it is destroyed at once, so a root that handles an event of the
/// popup window must touch nothing of itself after it dispatches the event to its widget (see `Root::handleEvent`).
struct PopupWindow: jwezel::Window, Root {
  ///
  /// Create a popup window
  ///
  /// @param terminal The terminal that supplies display and input.
  /// @param area Window area in display coordinates.
  /// @param background The window's background cell.
  /// @param owner The window the popup is transient to (weak reference), which keeps the terminal focus.
  /// @param colors Color style for the widgets of the popup, or nullptr for the default.
  /// @param style UI style for the widgets of the popup, or nullptr for the default.
  explicit PopupWindow(
    TerminalInterface &terminal,
    const Rectangle &area,
    const Char &background,
    jwezel::Window &owner,
    const ColorStyle *colors = nullptr,
    const UiStyle *style = nullptr
  );

  PopupWindow(const PopupWindow &) = delete;

  PopupWindow(PopupWindow &&) = delete;

  auto operator=(const PopupWindow &) -> PopupWindow & = delete;

  auto operator=(PopupWindow &&) -> PopupWindow & = delete;

  ~PopupWindow() override = default;

  /// A popup is placed to fit on the display; it does not grow it
  void ensure(const Rectangle &/*area*/) override {}

  ///
  /// The area of the popup widget, relative to the content of this window
  ///
  /// The window has no frame, so the content is the whole window; the area starts at its own origin.
  [[nodiscard]] auto content() const -> Rectangle override {
    return Rectangle{Vector{0, 0}, clientArea().size()};
  }
};

} // namespace jwezel::ui
