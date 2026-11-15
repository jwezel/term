#include "ui.hh"

#include <term/term.hh>
#include <term/window.hh>

namespace jwezel::ui {

Ui::Ui(Terminal &terminal):
terminal_{terminal},
desktop_{terminal.desktop()}
{}

void Ui::run() {
  terminal_.run();
}

void Ui::stop() {
  terminal_.stop();
}

} // namespace jwezel::ui
