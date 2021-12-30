#pragma once

#include "_screen_window.hh"
#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"
#include "text.hh"
#include <optional>
#include <string_view>

namespace jwezel {

struct Window
{
  ///
  /// Destroy window
  void destroy();

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area=RectangleMax);

  ///
  /// Get window area
  ///
  /// @return     Area
  Rectangle area() const;

  Vector size() const;

  Dim width() const;

  Dim height() const;

  ///
  /// Focus/unfocus window
  ///
  /// @param[in]  status  The status
  void focus(bool status=true);

  ///
  /// Write string to window
  ///
  /// @param[in]  str   The string
  void write(const Vector &position, const string_view &str);

  ///
  /// Write text to window
  ///
  /// @param[in]  text  The text
  void write(const Vector &position, const Text &text);

  ///
  /// Draw box in window
  ///
  /// @param[in]  box   The box
  void box(const Box &box={});

  ///
  /// Draw line in window
  ///
  /// @param[in]  line  The line
  void line(const Line &line={});

  ///
  /// Fill area in window
  ///
  /// @param[in]  fill  The fill character
  /// @param[in]  area  The area
  void fill(const Char &fill=Space, const Rectangle &area=RectangleMax);

  struct Terminal &terminal;  ///< Terminal
  screen::Window *window;     ///< Pointer to window
};

///
/// Terminal
///
/// Responsibilites:
///   - control display size
struct Terminal {
  using WinId = i2;

  ///
  /// Constructor
  ///
  /// @param[in]  initialPosition  The initial position
  /// @param[in]  initialSize      The initial size
  /// @param[in]  maxSize          The maximum size
  /// @param[in]  terminalFd       The terminal fd
  /// @param[in]  keyboardFd       The keyboard fd
  Terminal(
    const Char background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true
  );

  ///
  /// Create window
  ///
  /// @param[in]  area  The area
  ///
  /// @return     The window identifier.
  jwezel::Window newWindow(
    const Rectangle &area=RectangleDefault,
    const Char &background=Space,
    const optional<jwezel::Window> &below=nullopt
  );

  ///
  /// Delete window
  ///
  /// @param[in]  windowId  The window
  void deleteWindow(const jwezel::Window &window);

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(const jwezel::Window &window, const Rectangle &area);

  ///
  /// Set window focus
  ///
  /// @param[in]  window  The window
  /// @param[in]  status  The status
  void focus(const jwezel::Window &window, bool status=true);

  ///
  /// Write text to window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const jwezel::Window &window, const Vector &position, const Text &text);

  ///
  /// Draw box
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  position  The position
  /// @param[in]  box       The box
  void box(const jwezel::Window &window, const Box &box=Box{});

  ///
  /// Draw line
  ///
  /// @param[in]  window  The window
  /// @param[in]  line    The line
  void line(const jwezel::Window &window, const Line &line=Line{});

  ///
  /// Fill area
  ///
  /// @param[in]  window  The window
  /// @param[in]  fill    The fill
  /// @param[in]  area    The area
  void fill(const jwezel::Window &window, const Char &fill=Space, const Rectangle &area=RectangleMax);

  ///
  /// Get window area
  ///
  /// @param[in]  windowId  The window identifier
  ///
  /// @return     Window area
  Rectangle windowArea(const jwezel::Window &window) const;

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  bool expand(const Vector &size);

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  bool contract();

  Keyboard keyboard;
  Display display;
  Screen screen;
  screen::Window *desktop;
  Vector minimumSize;
  bool expand_;
  bool contract_;
};

} // namespace
