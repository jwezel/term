#pragma once

#include "geometry.hh"
#include "surface.hh"

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

};

}  // namespace jwezel
