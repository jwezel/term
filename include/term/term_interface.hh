#pragma once

#include "surface.hh"

namespace jwezel
{

struct TerminalInterface {
  virtual ~TerminalInterface() = default;

  virtual void registerWindow(struct Window *window) = 0;

  virtual auto screen() -> Surface & = 0;

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

}
