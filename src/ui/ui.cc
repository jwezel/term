#include "ui/ui.hh"
#include "term.hh"
#include "ui/window.hh"

#include <memory>

namespace jwezel::ui {

Ui::Ui(Terminal &terminal):
terminal_(terminal)
{}

void Ui::add(jwezel::ui::Window *window) {
  windows_.emplace(window, unique_ptr<Window>(window));
}
} // namespace jwezel::ui
