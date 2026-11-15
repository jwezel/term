#include "ui.hh"
#include "frame_style.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <memory>

namespace jwezel::ui {

namespace {

auto DefaultSize(Ui &ui, const Rectangle &area) {
  if (area == RectangleDefault) {
    return Rectangle{Vector{0, 0}, ui.terminal().display().maxSize() * 2 / 3};
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
jwezel::Window{&ui.terminal(), DefaultSize(ui, area), background},
Root{static_cast<jwezel::Window &>(*this)},
style_{std::make_unique<DefaultWindowFrameStyle>(this, title)},
title_{title}
{
  style_->draw();
}

void Window::move(const Rectangle &area) {
  Root::move(area);
}

} // namespace jwezel::ui
