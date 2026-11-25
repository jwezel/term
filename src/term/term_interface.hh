#pragma once

#include "geometry.hh"
#include "surface.hh"

#include <optional>

namespace jwezel
{

struct TerminalInterface {
  virtual ~TerminalInterface() = default;

  [[nodiscard]] virtual auto surface() -> Surface * = 0;

  ///
  /// Get the size of the display
  ///
  /// @return     The size of the display
  [[nodiscard]] virtual auto size() const -> Vector = 0;

  ///
  /// Get the size the display can grow to, which is the terminal size
  ///
  /// @return     The maximum display size
  [[nodiscard]] virtual auto maxSize() const -> Vector {return size();}

  virtual void focus(struct Window *window) = 0;

  ///
  /// Get the window that has the focus
  [[nodiscard]] virtual auto focus() const -> struct Window * {return nullptr;}

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  virtual auto expand(const Vector &size) -> bool = 0;

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  virtual auto contract() -> bool = 0;

  virtual void moveWindow(Window &window, const Rectangle &area) = 0;

  ///
  /// Place the text caret (the hardware cursor)
  ///
  /// Only the focused window controls the caret; requests of other windows
  /// are ignored. The caret is only shown while @a owner is the topmost
  /// element at the caret position, so a covered owner has no visible caret.
  ///
  /// @param      window    The requesting window
  /// @param      owner     The element the caret belongs to (weak reference)
  /// @param[in]  position  Display position of the caret, or nothing to hide it
  virtual void caret(
    Window &/*window*/,
    const Surface::Element */*owner*/,
    const std::optional<Vector> &/*position*/
  ) {}

};

}  // namespace jwezel
