#pragma once

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
  /// Move window
  ///
  /// @param[in]  area  The area
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
};

///
/// Window
struct Window: TextElement {
  explicit Window(struct TerminalInterface *terminal, const Rectangle &area, const Char &background = Space, Window *below = 0);

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
  /// Set the input handler (weak reference, nullptr for none)
  void inputHandler(InputHandler *handler) {inputHandler_ = handler;}

  [[nodiscard]] auto inputWindow() -> Window * override {return this;}

  [[nodiscard]] auto terminal() const {return terminal_;}

  private:
  struct TerminalInterface *terminal_;
  InputHandler *inputHandler_{};
};

} // namespace jwezel
