#pragma once

#include "term.hh"
#include "window.hh"
#include <memory>
#include <unordered_map>

namespace jwezel::ui {

struct Ui
{
  Ui(Terminal &terminal);

  ///
  /// Add window
  ///
  /// @param      window  The window
  void add(jwezel::ui::Window *window);

  Terminal &terminal;
  unordered_map<jwezel::ui::Window *, unique_ptr<jwezel::ui::Window>> windows;
};

} // namespace
