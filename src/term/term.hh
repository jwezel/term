#pragma once

#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"
#include "window.hh"

#include <optional>


namespace jwezel {

using std::nullopt;

///
/// Terminal
///
/// Responsibilites:
///   - Provide a:
///     - Display
///     - Keyboard
///   - Control display size
struct Terminal: Surface, TerminalInterface {
  ///
  /// Constructor
  ///
  /// @param[in]  background       The background
  /// @param[in]  initialPosition  The initial position
  /// @param[in]  initialSize      The initial size
  /// @param[in]  maxSize          The maximum size
  /// @param[in]  terminalFd       The terminal fd (default stdout)
  /// @param[in]  keyboardFd       The keyboard fd (default stdin)
  /// @param[in]  expand           Whether terminal can expand (default true)
  /// @param[in]  contract         Whether terminal can contract (default true)
  explicit Terminal(
    const Char &background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true
  );

  void addElement(Surface::Element *element, Surface::Element * below) override;

  void deleteElement(Element *element, Element *destination) override;

  ///
  /// Create window
  ///
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  /// @param[in]  below       The below
  ///
  /// @return     new window
  void focus(Window *window) override;

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(Window &window, const Rectangle &area) override;

  ///
  /// Get event
  ///
  /// @return     Event
  [[nodiscard]] auto event() -> Event;

  ///
  /// Run loop
  void run();

  void stop();

  [[nodiscard]] auto display() -> Display & {return display_;}

  [[nodiscard]] auto desktop() -> Window & {return desktop_;}

  [[nodiscard]] auto keyboard() -> Keyboard & {return keyboard_;}

  [[nodiscard]] auto surface() -> Surface * override {return this;}

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  auto expand(const Vector &size) -> bool override;

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  auto contract() -> bool override;

  private:
  bool expand_;
  bool contract_;
  Keyboard keyboard_;
  Display display_;
  Backdrop backdrop_;
  Window desktop_;
  Window *focusWindow_;
  Vector minimumSize_;
  bool running_;
};

} // namespace jwezel
