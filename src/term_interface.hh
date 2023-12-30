#pragma once

#include "surface.hh"

namespace jwezel
{

struct TerminalInterface {
  // TerminalInterface(const TerminalInterface &) = default;

  // TerminalInterface(TerminalInterface &&) = delete;

  // TerminalInterface &operator=(const TerminalInterface &) = default;

  // TerminalInterface &operator=(TerminalInterface &&) = delete;

  // virtual ~TerminalInterface() = default;

  virtual void registerWindow(struct Window *window) = 0;

  virtual auto screen() -> Surface::ref & = 0;

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
