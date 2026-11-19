#pragma once

#include "root.hh"

#include <term/term.hh>

#include <deque>
#include <functional>

namespace jwezel::ui {

/// Connects a terminal to a UI desktop and provides the UI event loop.
///
/// Windows created for this UI and widgets placed on its desktop use the same terminal event loop. The `Ui` object
/// holds a reference to its terminal, which must outlive it.
struct Ui {
  /// Work that the event loop runs after the current event has been handled.
  using Work = std::function<void()>;

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

  ///
  /// Post work to the event loop.
  ///
  /// The work is run after the event that is currently being handled has been handled, before the next event. It
  /// is the way to ask the event loop to do something that is not an event: close a dialog, return the focus to a
  /// window below, or stop.
  ///
  /// @param[in]  work  The work to run
  void post(const Work &work);

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
  std::deque<Work> work_;
};

} // namespace jwezel::ui
