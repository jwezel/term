#pragma once

#include "action.hh"
#include "color_style.hh"
#include "container.hh"
#include "ui_style.hh"

#include <term/geometry.hh>
#include <term/mouse_capture.hh>
#include <term/window.hh>

#include <vector>

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
/// Keys that no widget has made use of are for the window: first the actions that were added to the focused widget,
/// then the actions that were added to the window, and then Tab and Shift-Tab, which move the focus to the next or
/// previous widget that can have it (see `FocusPolicy`). The order is the one in which the widgets were created,
/// which `tabOrder` can change.
///
/// All areas of the elements in a root are relative to the host window. The
/// root always follows the size and position of the host window: when it
/// changes, the elements are laid out again. Elements that were created
/// without an area fill the content area of the root, the others keep their
/// area.
struct Root: Container, InputHandler, ActionHost {
  ///
  /// Create a UI root hosted by an existing terminal window.
  /// @param host Window that supplies input, geometry, and the underlying display surface; it must outlive the root.
  /// @param colors Optional non-owning color style; it must outlive the root.
  /// @param style Optional non-owning glyph/decoration style; it must outlive the root.
  explicit Root(jwezel::Window &host, const ColorStyle *colors = nullptr, const UiStyle *style = nullptr);

  Root(const Root &) = delete;

  Root(Root &&) = delete;

  auto operator=(const Root &) -> Root & = delete;

  auto operator=(Root &&) -> Root & = delete;

  ~Root() override;

  [[nodiscard]] auto host() const -> jwezel::Window & {return host_;}

  ///
  /// Whether the host window has the terminal focus, which makes its focused widget the one that gets the input
  [[nodiscard]] auto active() const -> bool;

  ///
  /// Get the colors of the elements: the color style of the root, or the default one
  [[nodiscard]] auto colors() const -> const ColorStyle &;

  ///
  /// Set the color style (weak reference, nullptr for the default); all elements are drawn again
  void colors(const ColorStyle *colors);

  ///
  /// Get the style of the elements: the UI style of the root, or the default one
  [[nodiscard]] auto style() const -> const UiStyle &;

  ///
  /// Set the UI style (weak reference, nullptr for the default); all elements are drawn again
  void style(const UiStyle *style);

  ///
  /// Draw all elements again, because the colors or the style have changed
  virtual void restyle();

  ///
  /// Get the position of the host window on the display
  [[nodiscard]] auto origin() const -> Vector {return host_.area().position();}

  [[nodiscard]] auto root() -> Root * override {return this;}

  ///
  /// Get the area that elements without an area of their own fill
  ///
  /// By default this is the whole host window, the area of the desktop for example.
  ///
  /// @return     The content area, relative to the host window
  [[nodiscard]] virtual auto content() const -> Rectangle {return Rectangle{Vector{0, 0}, host_.area().size()};}

  auto add(Element &child) -> Container & override;

  ///
  /// Focus a widget
  ///
  /// @param      widget  The widget (weak reference), or nullptr for none
  void focus(Widget *widget);

  [[nodiscard]] auto focus() const -> Widget * {return focus_;}

  ///
  /// Move the focus to the next widget that can have it with Tab, wrapping around at the end
  void focusNext();

  ///
  /// Move the focus to the previous widget that can have it with Tab, wrapping around at the beginning
  void focusPrevious();

  ///
  /// Get the widgets in the order the focus moves through them
  [[nodiscard]] auto tabChain() const -> const std::vector<Widget *> & {return chain_;}

  ///
  /// Change the order of the focus: the second widget is moved to come right after the first one
  ///
  /// Chaining the widgets of a window two by two defines the whole order, like in Qt.
  ///
  /// @throws     std::invalid_argument if one of them is not in this root
  void tabOrder(Widget &first, Widget &second);

  ///
  /// Change the order of the focus: the widgets come one after the other, in this order, the first of them
  /// staying where it is
  void tabOrder(const std::vector<Widget *> &order);

  ///
  /// Add a widget to the order of the focus, after all others; called when a widget is created
  void enroll(Widget &widget);

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

  ///
  /// Place all elements again, relative to the host window as it is now
  void layout();

  [[nodiscard]] auto bounds() -> Rectangle override {return host_.area();}

  /// Route an input event to the appropriate widget or action.
  auto handleEvent(const Event &event) -> bool override;

  void handleFocus() override;

  void handleBlur() override;

  void handleGeometry(const Rectangle &previous) override;

  private:
  void step(int direction);

  auto handleKey(const Event &event) -> bool;

  std::vector<Widget *> chain_; // weak references, in order of the focus
  jwezel::Window &host_;
  const ColorStyle *colors_;  // weak reference, nullptr for the default
  const UiStyle *style_;      // weak reference, nullptr for the default
  Widget *focus_{}; // weak reference to the focused widget
  Widget *grab_{};  // weak reference to the widget that got the mouse press that is still going on
  MouseCapture capture_;
};

} // namespace jwezel::ui
