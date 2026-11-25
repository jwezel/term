#pragma once

#include "element_device.hh"
#include "geometry.hh"
#include "surface.hh"
#include "term/event.hh"
#include "term_interface.hh"
#include "text.hh"

namespace jwezel {

///
/// A virtually infinite window, all blank with no buffer, used to represent
/// the display in a clear state.
struct Backdrop: public Surface::Element {

  explicit Backdrop(Surface *surface);

  ///
  /// Destroy Backdrop
  ~Backdrop() override = default;

  Backdrop(const Backdrop &) = delete;

  Backdrop(Backdrop &&) = delete;

  auto operator=(const Backdrop &) -> Backdrop & = default;

  auto operator=(Backdrop &&) -> Backdrop & = delete;

  ///
  /// Get backdrop area
  ///
  /// @return     RectangleMax
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area=RectangleMax) const -> Text override;

  ///
  /// Ignore attempts to move the backdrop.
  void move(const Rectangle &/*area*/) override {}
};

///
/// Receiver of the input and focus events of a window
///
/// Lets behavior be attached to a window that cannot be derived from, like
/// the desktop. The handler is a weak reference: the handler is owned by its
/// creator and has to unregister before the window goes away.
struct InputHandler {
  InputHandler() = default;

  InputHandler(const InputHandler &) = delete;

  InputHandler(InputHandler &&) = delete;

  auto operator=(const InputHandler &) -> InputHandler & = delete;

  auto operator=(InputHandler &&) -> InputHandler & = delete;

  virtual ~InputHandler() = default;

  ///
  /// Handle an input event
  ///
  /// @param[in]  event  The event
  ///
  /// @return     True if the event was handled
  virtual auto handleEvent(const Event &event) -> bool = 0;

  ///
  /// The window received the terminal focus
  virtual void handleFocus() {}

  ///
  /// The window lost the terminal focus
  virtual void handleBlur() {}

  ///
  /// The window was moved or resized.
  virtual void handleGeometry(const Rectangle &/*previous*/) {}
};

///
/// Window
/// A rectangular, z-ordered region of a surface with its own input and focus routing.
///
/// A window is drawn above the elements below it. Its terminal coordinates are managed by `Terminal`. A window is
/// also an element device: elements on its client surface are clipped to the window's client area and move with it,
/// which is how a UI lives inside a window. The client area is the window minus its insets, which a decorated window
/// sets so that its widgets keep clear of the frame.
///
/// A window can be transient to another one, like a popup that belongs to a window: the transient never takes the
/// terminal focus itself, which its owner keeps, and while the owner has the focus the terminal routes input that
/// lands on the transient to it.
///
/// A window with a frame can be managed with the mouse: pressing the title bar drags it, and pressing the left,
/// right, or bottom border resizes it. The title bar is the part above the client area; the other frame parts resize
/// from their edge, and corners resize both axes. A gesture stays within the display and keeps at least one client
/// cell. An undecorated window has no frame, so it has nothing to grab.
struct Window: ElementDevice {
  Window(
    struct TerminalInterface *terminal,
    const Rectangle &area,
    const Char &background = Space,
    Window *below = 0,
    Window *transient = 0
  );

  ~Window() override;

  Window(const Window &) = delete;

  Window(Window &&) = delete;

  auto operator=(const Window &) -> Window & = delete;

  auto operator=(Window &&) -> Window & = delete;

  explicit operator string() const;

  void move(const Rectangle &area) override;

  ///
  /// Handle an input event; goes to the input handler, if there is one
  virtual auto event(const Event &event) -> bool;

  ///
  /// Called when the window has received the terminal focus
  virtual void focusEvent();

  ///
  /// Called when the window has lost the terminal focus
  virtual void blurEvent();

  ///
  /// Called when the window has been moved or resized
  ///
  /// @param[in]  previous  The area the window had before
  virtual void geometryEvent(const Rectangle &previous);

  ///
  /// Set the input handler (weak reference, nullptr for none)
  void inputHandler(InputHandler *handler) {inputHandler_ = handler;}

  ///
  /// Get the device the widgets of a UI in this window are drawn into
  ///
  ///
  /// Set how much of the window the content area keeps clear of, on each side
  ///
  /// A decorated window sets the insets of its decoration, so that the UI places its widgets inside the frame.
  ///
  /// @param[in]  insets  The insets, from the edges of the window
  void clientInsets(const Insets &insets);

  ///
  /// Get how much of the window the content area keeps clear of, on each side
  [[nodiscard]] auto clientInsets() const -> const Insets & {return insets_;}

  ///
  /// Get the content area, relative to the window
  ///
  /// @return     The area inside the insets, clamped to the window
  [[nodiscard]] auto clientArea() const -> Rectangle;

  ///
  /// Make this window transient to another one (weak reference, nullptr for none)
  ///
  /// A transient window is a popup-like window that belongs to its owner: the owner keeps the terminal focus, and
  /// while it has it the transient gets the input that lands on it.
  ///
  /// @param[in]  owner  The window the transient belongs to
  void transient(Window *owner) {transient_ = owner;}

  ///
  /// Get the window this one is transient to, or nullptr if it is not
  [[nodiscard]] auto transient() const -> Window * {return transient_;}

  [[nodiscard]] auto inputWindow() -> Window * override {return this;}

  [[nodiscard]] auto terminal() const {return terminal_;}

  ///
  /// Whether the window takes the input exclusively while it has the focus: a modal window, which no other window
  /// gets input while it is open and focused.
  [[nodiscard]] auto modal() const -> bool {return modal_;}

  ///
  /// Set whether the window takes the input exclusively while it has the focus
  ///
  /// @param[in]  modal  True for a modal window, which gets the input while it is open, and no other window gets it
  ///                    while it has the focus
  void modal(bool modal) {modal_ = modal;}

  private:
  [[nodiscard]] auto beginGesture(const Vector &position) -> bool;

  auto continueGesture(const Vector &position) -> bool;

  struct TerminalInterface *terminal_;
  InputHandler *inputHandler_{};
  bool modal_{false};
  Window *transient_{}; // weak reference; the window this one is transient to
  Insets insets_;
  bool moving_{false};          // whether a drag is in progress
  int resizeX_{0};              // -1 for the left border, 1 for the right one, 0 for neither
  int resizeY_{0};              // 1 for the bottom border, 0 for neither
  Vector gesturePress_;         // where the gesture started, in display coordinates
  Rectangle gestureArea_;       // the window area when the gesture started
};

} // namespace jwezel
