#include "surface.hh"
#include "ui.hh"
#include "geometry.hh"
#include "text.hh"
#include "widget.hh"

#include <fmt/core.h>

using namespace jwezel::ui;

jwezel::ui::Window::Window(Ui *ui, const Char &background, const Rectangle &area):
Widget(0, background, area.size()),
Surface(),
window_{ui->terminal.newWindow(area, background)}
{
  if (ui) {
    ui->add(this);
  }
}

Rectangle jwezel::ui::Window::area() const {
  return window_.area();
}

Vector jwezel::ui::Window::size() const {
  return window_.size();
}

Dim jwezel::ui::Window::width() const {
  return window_.width();
}

Dim jwezel::ui::Window::height() const {
  return window_.height();
}

void jwezel::ui::Window::update(const Updates &updates) {
  for (auto update: updates) {
    // fmt::print(stderr, "{}: {} {}\n", string(update.position), update.text.repr(), update.text.height()? string(update.text.at({0, 0})): "-");
    window_.write(update.position, update.text);
  }
}
