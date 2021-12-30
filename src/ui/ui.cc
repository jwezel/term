#include "ui.hh"
#include "term.hh"
#include "window.hh"
#include <memory>

using namespace jwezel::ui;

Ui::Ui(Terminal &terminal):
terminal(terminal)
{}

void Ui::add(jwezel::ui::Window *window) {
  windows.insert(make_pair(window, unique_ptr<Window>(window)));
}
