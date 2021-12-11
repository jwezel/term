
#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "screen.hh"
#include "window.hh"

namespace jwezel {


///
/// Terminal
///
/// Responsibilites:
///   - control display size
struct Terminal {
  using WinId = u2;

  ///
  /// Constructor
  ///
  /// @param[in]  initialPosition  The initial position
  /// @param[in]  initialSize      The initial size
  /// @param[in]  maxSize          The maximum size
  /// @param[in]  terminalFd       The terminal fd
  /// @param[in]  keyboardFd       The keyboard fd
  Terminal(
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0
  );

  ///
  /// Create window
  ///
  /// @param[in]  area  The area
  ///
  /// @return     The window identifier.
  WinId newWindow(const Rectangle &area=RectangleDefault, WinId below=-1);

  ///
  /// Delete window
  ///
  /// @param[in]  windowId  The window
  void deleteWindow(WinId windowId);

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(WinId windowId, const Rectangle &area);

  ///
  /// Write text to window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(WinId windowId, const Vector &position, const Text &text);

  ///
  /// Get window area
  ///
  /// @param[in]  windowId  The window identifier
  ///
  /// @return     Window area
  Rectangle windowArea(WinId windowId) const;

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  area  The area
  void expand(const Rectangle &area);

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  area  The area
  void contract(const Rectangle &area);

  Keyboard keyboard;
  Display display;
  Screen screen;
};

} // namespace