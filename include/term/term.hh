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

using std::nullopt;

struct Window
{
  Window(struct Terminal &terminal, screen::Window *window): terminal_(terminal), window_(window) {}

  ///
  /// Destroy window
  void destroy();

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area=RectangleMax);

  [[nodiscard]] inline auto window() const {
    return window_;
  }

  ///
  /// Get window area
  ///
  /// @return     Area
  [[nodiscard]] auto area() const -> Rectangle;

  [[nodiscard]] auto size() const -> Vector;

  [[nodiscard]] auto width() const -> Dim;

  [[nodiscard]] auto height() const -> Dim;

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
  void box(const Box &box=Box{});

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

  private:
  struct Terminal &terminal_;  ///< Terminal
  screen::Window *window_;     ///< Pointer to window
};

///
/// Terminal
///
/// Responsibilites:
///   - Provide a:
///     - Display
///     - Keyboard
///   - Control display size
struct Terminal {
  using WinId = i2;

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
    Char background=Space,
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
  auto newWindow(
    const Rectangle &area=RectangleDefault,
    const Char &background=Space,
    const optional<jwezel::Window> &below=nullopt
  ) -> jwezel::Window;

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
  /// Get event
  ///
  /// @return     Event
  auto event() -> Event *;

  ///
  /// Run loop
  void run();

  auto display() -> Display & {return display_;}

  auto desktop() -> screen::Window & {return *desktop_;}

  auto keyboard() -> Keyboard & {return keyboard_;}

  protected:
  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  auto expand(const Vector &size) -> bool;

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  auto contract() -> bool;

  private:
  Keyboard keyboard_;
  Display display_;
  screen::Screen screen_;
  screen::Window *desktop_;
  Vector minimumSize_;
  bool expand_;
  bool contract_;
  bool running_;
};

} // namespace jwezel
