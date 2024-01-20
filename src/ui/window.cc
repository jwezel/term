#include "ui.hh"
#include "frame_style.hh"
#include "ui/widget.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <memory>

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
title_{title},
focus_{0}
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

void Window::focus(Widget *widget) {
  focus_ = widget;
}

auto Window::focus() -> Widget * {
 return focus_;
}

auto Window::defaultFocus() -> Widget * {
 if (!focus_ && !surface_.zorder().empty()) {
    return dynamic_cast<Widget *>(surface_.zorder().front());
 }
 return focus_;
}

auto Window::event(const jwezel::Event &event) -> bool {
  if (focus_) {
    return focus_->event(event);
  }
  return false;
}

} // namespace jwezel::ui
