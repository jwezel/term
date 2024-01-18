#include "ui.hh"
#include <term/term.hh>
#include <term/window.hh>


namespace jwezel::ui {

Ui::Ui(Terminal *terminal):
terminal_(terminal)
{}

void Ui::run() {
  terminal_->run();
}

void Ui::add(jwezel::ui::Window *window) {
  windows_.insert(window);
}

} // namespace jwezel::ui
