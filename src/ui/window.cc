#include "ui.hh"
#include "frame_style.hh"
#include "ui/widget.hh"
#include "window.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <memory>
#include <utility>
#include <vector>

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
Container{nullptr},
style_{std::make_unique<DefaultWindowFrameStyle>(this, title)},
title_{title},
focus_{nullptr}
{
  style_->draw();
}

void Window::focus(Widget *widget) {
  focus_ = widget;
  updateCaret();
}

void Window::focusEvent() {
  updateCaret();
}

void Window::updateCaret() {
  terminal()->caret(*this, focus_, focus_? focus_->caret(): std::nullopt);
}

auto Window::focus() -> Widget * {
 return focus_;
}

void Window::move(const Rectangle &area) {
  std::vector<std::pair<ui::Element *, Rectangle>> placements;
  for (auto *child: children()) {
    placements.emplace_back(child, child->bounds());
  }
  jwezel::Window::move(area);
  for (const auto &[child, relative]: placements) {
    child->move(relative);
  }
  updateCaret();
}

auto Window::event(const jwezel::Event &event) -> bool {
  if (event.type() == MouseMoveEvent::type_ or event.type() == MouseButtonEvent::type_) {
    const auto position{dynamic_cast<MouseEvent *>(event())->position()};
    Surface::Fragment *fragment{};
    try {
      fragment = surface()->find(position);
    } catch (const std::logic_error &) { // no single topmost element at this position
      fragment = 0;
    }
    // Mouse input only concerns what is visible at the position: nothing covered
    // by another element, and nothing of another window
    auto *element{fragment? fragment->element: nullptr};
    auto *widget{dynamic_cast<Widget *>(element)};
    if (element != static_cast<jwezel::Window *>(this) and (!widget or widget->window() != this)) {
      return false;
    }
    if (
      event.type() == MouseButtonEvent::type_ and
      dynamic_cast<MouseButtonEvent *>(event())->action() == MouseAction::Down
    ) {
      // Click to focus: the widget that was hit and the window it is in
      if (widget) {
        focus(widget);
      }
      terminal()->focus(this);
    }
    return widget? widget->event(event): false;
  }
  // Other input goes to the focused widget, unless it is completely covered
  if (focus_ and !focus_->fragments().empty()) {
    return focus_->event(event);
  }
  return false;
}

} // namespace jwezel::ui
