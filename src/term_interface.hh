#pragma once

namespace jwezel
{

struct TerminalInterface {
  virtual void registerWindow(struct BaseWindow *window) = 0;

  virtual auto screen() -> struct Surface & = 0;

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  virtual auto expand(const struct Vector &size) -> bool = 0;

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  virtual auto contract() -> bool = 0;

  virtual void moveWindow(struct BaseWindow &window, const struct Rectangle &area) = 0;

};

}  // namespace jwezel
