#include "term/surface.hh"
#include "ui.hh"
#include "ui/frame_style.hh"
#include "window.hh"

#include <memory>
#include <term/geometry.hh>
#include <term/window.hh>

namespace jwezel::ui {

namespace {

auto DefaultSize(Ui &ui, const Rectangle &area) {
  if (area == RectangleDefault) {
    return Rectangle{Vector{0, 0}, ui.terminal()->display().maxSize() * 2 / 3};
  }
  return area;
}

} // namespace

struct DefaultWindowFrameStyle: FrameStyle {
  explicit DefaultWindowFrameStyle(TextElement *window, const Text& title={}):
  window_{window},
  title_{title}
  {}

  void draw() override {
    window_->box();
    window_->write(Vector{1, 0}, title_);
  }

  private:
  TextElement *window_;
  Text title_;
};

Window::Window(Ui &ui, const Char &background, const Rectangle &area, const Text &title):
jwezel::Window{ui.terminal(), DefaultSize(ui, area), background},
Container{0},
style_{new DefaultWindowFrameStyle{this, title}},
device_{this},
surface_{&device_},
title_{title}
{
  style_->draw();
}

Window::Device::Device(Window *window):
window_{window}
{}

void Window::Device::update(const jwezel::Updates &updates) {
  for (const auto &update_ : updates) {
    window_->write(update_.position, update_.text);
  }
}

} // namespace jwezel::ui
