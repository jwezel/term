#include "ui.hh"
#include "geometry.hh"
#include "text.hh"

using namespace jwezel::ui;

jwezel::ui::Window::Window(Ui *ui, const Char &background, const Rectangle &area):
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

}
