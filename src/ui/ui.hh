#pragma once

#include "root.hh"

#include <term/term.hh>

namespace jwezel::ui {

struct Ui {
  explicit Ui(Terminal &terminal);

  Ui(const Ui &) = delete;

  Ui(Ui &&) = delete;

  auto operator=(const Ui &) -> Ui & = delete;

  auto operator=(Ui &&) -> Ui & = delete;

  ~Ui() = default;

  void run();

  void stop();

  [[nodiscard]] auto terminal() const -> auto & {return terminal_;}

  ///
  /// Get the desktop
  ///
  /// The desktop is a window like any other, where a UI can be placed. Small
  /// applications that do not need windows put their elements right on it.
  /// Areas on the desktop are display positions.
  ///
  /// @return     The root of the UI on the desktop
  [[nodiscard]] auto desktop() -> Root & {return desktop_;}

  private:
  Terminal &terminal_;
  Root desktop_;
};

} // namespace jwezel::ui
