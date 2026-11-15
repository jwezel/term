#pragma once

#include "geometry.hh"
#include "surface.hh"

#include <optional>

namespace jwezel
{

struct TerminalInterface {
  virtual ~TerminalInterface() = default;

  [[nodiscard]] virtual auto surface() -> Surface * = 0;

  virtual void focus(struct Window *window) = 0;

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
