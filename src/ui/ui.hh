#pragma once

#include "window.hh"

#include <term/term.hh>
#include <term/window.hh>
#include <unordered_set>

namespace jwezel::ui {

struct Ui {
  explicit Ui(Terminal *terminal);

  void run();

  ///
  /// Add window
  ///
  /// @param      window  The window
  void add(Window *window);

  inline auto terminal() -> auto & {return terminal_;}

  private:
  Terminal *terminal_;
  std::unordered_set<jwezel::ui::Window *> windows_;
  bool running_ = false;
};

} // namespace jwezel::ui
