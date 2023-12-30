#pragma once

#include "term.hh"
#include "window.hh"
#include <memory>
#include <unordered_map>

namespace jwezel::ui {

struct Ui {
  explicit Ui(Terminal &terminal);

  ///
  /// Add window
  ///
  /// @param      window  The window
  void add(jwezel::ui::Window *window);

  inline auto terminal() -> auto & {return terminal_;}

  private:
  Terminal &terminal_;
  unordered_map<jwezel::ui::Window *, unique_ptr<jwezel::ui::Window>> windows_;
};

} // namespace jwezel::ui
