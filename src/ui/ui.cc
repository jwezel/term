#include "ui.hh"

#include <term/term.hh>
#include <term/window.hh>

namespace jwezel::ui {

Ui::Ui(Terminal &terminal):
terminal_{terminal},
desktop_{terminal.desktop()}
{}

void Ui::run() {
  terminal_.start();
  while (terminal_.running()) {
    terminal_.runEvent();
    // Work that was posted during the event is run after it, before the next event.
    auto work{std::move(work_)};
    work_.clear();
    for (auto &item: work) {
      item();
    }
  }
}

void Ui::stop() {
  terminal_.stop();
}

void Ui::post(const Ui::Work &work) {
  work_.push_back(work);
}

} // namespace jwezel::ui
