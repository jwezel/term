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
  /// are ignored.
  ///
  /// @param      window    The requesting window
  /// @param[in]  position  Display position of the caret, or nothing to hide it
  virtual void caret(Window &/*window*/, const std::optional<Vector> &/*position*/) {}

};

}  // namespace jwezel
