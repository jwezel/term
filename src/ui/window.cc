#include "controller.hh"
#include "layout_container.hh"
#include "ui.hh"
#include "geometry.hh"
#include "text.hh"
#include "widget.hh"

using namespace jwezel::ui;

jwezel::ui::Window::Window(Ui *ui, const Char &background, const Rectangle &area):
Controller{0, area == RectangleDefault? Rectangle{{0, 0}, ui->terminal.display.size() * (2. / 3.)}: area},
window{ui->terminal.newWindow(area, background)}
{
  if (ui) {
    ui->add(this);
  }
}

Rectangle jwezel::ui::Window::area() const {
  return window.area();
}

Vector jwezel::ui::Window::size() const {
  return window.size();
}

Dim jwezel::ui::Window::width() const {
  return window.width();
}

Dim jwezel::ui::Window::height() const {
  return window.height();
}
