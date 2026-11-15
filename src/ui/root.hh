#pragma once

#include "container.hh"

#include <term/geometry.hh>
#include <term/mouse_capture.hh>
#include <term/window.hh>

namespace jwezel::ui {

struct Widget;

///
/// Root of a tree of UI elements, shown in a window
///
/// The window it is shown in is its host, which can be any window: one that is
/// derived from (see `ui::Window`) or an existing one, like the terminal's
/// desktop. A root is a normal container, so a small application can place its
/// UI right on the desktop and do without windows.
///
/// The root keeps track of the focused widget and its caret, hands the input
/// of the host window to the right widgets and lets the host window claim the
/// terminal focus when it is clicked. The root is the creator of nothing: all
/// elements are owned by whoever created them, and must be destroyed before
/// the root.
///
/// All areas of the elements in a root are relative to the host window.
struct Root: Container, InputHandler {
  ///
  /// Create Root
  ///
  /// @param      host  The window the UI is shown in; it must outlive the root
  explicit Root(jwezel::Window &host);

  Root(const Root &) = delete;

  Root(Root &&) = delete;

  auto operator=(const Root &) -> Root & = delete;

  auto operator=(Root &&) -> Root & = delete;

  ~Root() override;

  [[nodiscard]] auto host() const -> jwezel::Window & {return host_;}

  ///
  /// Get the position of the host window on the display
  [[nodiscard]] auto origin() const -> Vector {return host_.area().position();}

  [[nodiscard]] auto root() -> Root * override {return this;}

  ///
  /// Focus a widget
  ///
  /// @param      widget  The widget (weak reference), or nullptr for none
  void focus(Widget *widget);

  [[nodiscard]] auto focus() const -> Widget * {return focus_;}

  ///
  /// Tell the terminal where the caret of the focused widget is (or hide it)
  void updateCaret();

  ///
  /// Forget a widget that is about to be destroyed
  void forget(Widget &widget);

  ///
  /// Move the host window on the display; the elements keep their areas
  /// relative to it and follow
  void move(const Rectangle &area) override;

  [[nodiscard]] auto bounds() -> Rectangle override {return host_.area();}

  auto handleEvent(const Event &event) -> bool override;

  void handleFocus() override;

  private:
  jwezel::Window &host_;
  Widget *focus_{}; // weak reference to the focused widget
  Widget *grab_{};  // weak reference to the widget that got the mouse press that is still going on
  MouseCapture capture_;
};

} // namespace jwezel::ui
