#pragma once

#include <term/term.hh>

namespace jwezel::ui {

struct Ui {
  explicit Ui(Terminal &terminal);

  void run();

  void stop();

  [[nodiscard]] auto terminal() const -> auto & {return terminal_;}

  private:
  Terminal &terminal_;
};

} // namespace jwezel::ui
