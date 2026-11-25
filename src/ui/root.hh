#pragma once

#include "action.hh"
#include "color_style.hh"
#include "container.hh"
#include "ui_style.hh"

#include <term/geometry.hh>
#include <term/element_device.hh>
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
/// All areas of the elements in a root are relative to the host window's content area, which is the client
/// area of the window: the frame is not part of it, so a window that frames its content keeps the widgets out of
/// its frame. The root always follows the size and position of the host window: when it
/// changes, the elements are laid out again. Elements that were created
/// without an area fill the content area of the root, the others keep their area.
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
  /// Get the device the elements of this root are drawn into
  ///
  /// It is the host window itself: an element device whose content surface holds the widgets.
  [[nodiscard]] auto client() -> ElementDevice & {return host_;}
  [[nodiscard]] auto client() const -> const ElementDevice & {return host_;}

  ///
  /// Get the content area, in content-relative coordinates: the content starts at its own origin, which the
  /// area position says, and the widgets are clipped to it from there
  ///
  /// By default this is the client area of the host window, which is the whole host window for the desktop and
  /// the content of a window for a window-hosted root.
  ///
  /// @return     The content area, relative to the content origin
  [[nodiscard]] virtual auto content() const -> Rectangle {
    return Rectangle{Vector{0, 0}, host_.clientArea().size()};
  }

  /// Get the display position where the content area starts
  ///
  /// The content area starts there, which the frame of a window puts after its insets.
  ///
  /// @return     The position of the content area on the display
  [[nodiscard]] auto contentOrigin() const -> Vector {return host_.clientArea().position();}

  /// Get the display position of the content origin
  ///
  /// @return     The position on the display where the content area starts
  [[nodiscard]] auto displayOrigin() const -> Vector {return host_.area().position() + contentOrigin();}

  ///
  /// Make sure the host window is large enough to show an area of the content
  ///
  /// The terminal grows the host window and the display when the area needs more room. A widget that is moved
  /// calls this, so a layout or an explicit move can grow the window it is in.
  ///
  /// @param[in]  area  The area, relative to the content
  virtual void ensure(const Rectangle &area);

  ///
  /// Whether any part of the widget shows on the display
  ///
  /// A widget is covered where an element of its own window covers it, which its fragments already say, and where
  /// another window covers the window. A host-placed widget is already on the surface of the host window.
  ///
  /// @param[in]  widget  The widget
  ///
  /// @return     Whether any part of the widget is visible
  [[nodiscard]] auto visible(const Widget &widget) const -> bool;

  auto add(Element &child) -> Container & override;

  // A root hosts actions as well as it contains elements: both names have to stay reachable
  using ActionHost::add;
  using Container::remove;
  using ActionHost::remove;

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
