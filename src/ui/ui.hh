#pragma once

#include "root.hh"

#include <term/term.hh>

namespace jwezel::ui {

/// Connects a terminal to a UI desktop and provides the UI event loop.
///
/// Windows created for this UI and widgets placed on its desktop use the same terminal event loop. The `Ui` object
/// holds a reference to its terminal, which must outlive it.
struct Ui {
  /// Create a UI for a terminal.
  /// @param terminal The terminal that supplies display and input.
  explicit Ui(Terminal &terminal);

  Ui(const Ui &) = delete;

  Ui(Ui &&) = delete;

  auto operator=(const Ui &) -> Ui & = delete;

  auto operator=(Ui &&) -> Ui & = delete;

  ~Ui() = default;

  /// Run the terminal event loop until `stop()` is called.
  void run();

  /// Ask the event loop started by `run()` to stop.
  void stop();

  /// Get the terminal connected to this UI.
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
