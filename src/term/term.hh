#pragma once

#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "mouse_capture.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"
#include "window.hh"

#include <optional>


namespace jwezel {

using std::nullopt;

///
/// Coordinates terminal input, display output, windows, and the event loop.
///
/// The terminal owns its `Display`, `Keyboard`, backdrop, and desktop window. The selected screen behavior is
/// controlled by `Display::Screen`.
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
  /// @param[in]  screen           The screen to use: the main screen, on which the display makes room for itself
  ///                              below the cursor by scrolling the terminal, or the alternate screen, which is
  ///                              given up again at the end
  explicit Terminal(
    const Char &background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true,
    Display::Screen screen=Display::Screen::main
  );

  void addElement(Surface::Element *element, Surface::Element * below) override;

  void deleteElement(Element *element, Element *destination) override;

  ///
  /// Set the window that receives terminal focus.
  ///
  /// @param[in]  window  The window, or `nullptr` to clear focus
  void focus(Window *window) override;

  ///
  /// Move a window to a new display area.
  ///
  /// @param[in]  window  The window to move
  /// @param[in]  area    The new area
  void moveWindow(Window &window, const Rectangle &area) override;

  void caret(Window &window, const Surface::Element *owner, const std::optional<Vector> &position) override;

  void reshapeElement(Element *element, const Rectangle &area) override;

  ///
  /// Get event
  ///
  /// @return     Event
  [[nodiscard]] auto event() -> Event;

  void runEvent();

  ///
  /// Start the event loop
  void start();

  ///
  /// Run loop
  ///
  /// Runs until `stop()` is called
  void run();

  ///
  /// Ask the event loop to stop
  void stop();

  ///
  /// Get whether the event loop is running
  ///
  /// @return     True if the event loop is running
  [[nodiscard]] auto running() const -> bool {return running_;}

  [[nodiscard]] auto focus() const -> Window * override {return focusWindow_;}

  [[nodiscard]] auto display() -> Display & {return display_;}

  [[nodiscard]] auto desktop() -> Window & {return desktop_;}

  [[nodiscard]] auto keyboard() -> Keyboard & {return keyboard_;}

  [[nodiscard]] auto surface() -> Surface * override {return this;}

  ///
  /// Possibly expand display and screen
  ///
  /// The display grows to at least the size, up to its maximum size. On the main screen it scrolls the terminal
  /// up if there is not enough room below it.
  ///
  /// @param[in]  size  The size
  auto expand(const Vector &size) -> bool override;

  ///
  /// Reduce the display to fit the remaining elements, if possible.
  ///
  /// @return     Whether the display was contracted
  auto contract() -> bool override;

  private:
  ///
  /// Show the requested caret if its owner is visible there, hide it otherwise
  void placeCaret();

  ///
  /// Get the window that is visible at a display position
  ///
  /// @param[in]  position  The position
  ///
  /// @return     The window, or nothing if there is none (e.g. the backdrop)
  auto windowAt(const Vector &position) -> Window *;

  const Surface::Element *caretOwner_{};
  std::optional<Vector> caretPosition_;
  Window *focusWindow_{}; // weak reference; set by the first window that is created, the desktop
  Window *captureWindow_{}; // weak reference; the window that got the mouse press that is still going on
  MouseCapture mouseCapture_;
  bool expand_;
  bool contract_;
  Keyboard keyboard_;
  Display display_;
  Backdrop backdrop_;
  Window desktop_;
  Vector minimumSize_;
  bool running_;
};

} // namespace jwezel
