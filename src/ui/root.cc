#include "root.hh"
#include "widget.hh"

#include <term/keyboard.hh>
#include <term/surface.hh>
#include <term/term_interface.hh>

#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace jwezel::ui {

Root::Root(jwezel::Window &host):
Container{nullptr},
host_{host}
{
  host_.inputHandler(this);
}

Root::~Root() {
  host_.inputHandler(nullptr);
}

void Root::focus(Widget *widget) {
  focus_ = widget;
  updateCaret();
}

void Root::updateCaret() {
  host_.terminal()->caret(host_, focus_, focus_? focus_->caret(): std::nullopt);
}

void Root::forget(Widget &widget) {
  if (grab_ == &widget) {
    grab_ = nullptr;
  }
  if (focus_ == &widget) {
    focus(nullptr);
  }
}

void Root::move(const Rectangle &area) {
  std::vector<std::pair<ui::Element *, Rectangle>> placements;
  for (auto *child: children()) {
    placements.emplace_back(child, child->bounds());
  }
  host_.jwezel::Window::move(area);
  for (const auto &[child, relative]: placements) {
    child->move(relative);
  }
  updateCaret();
}

void Root::handleFocus() {
  updateCaret();
}

auto Root::handleEvent(const Event &event) -> bool {
  if (event.type() == MouseMoveEvent::type_ or event.type() == MouseButtonEvent::type_) {
    const auto position{dynamic_cast<MouseEvent *>(event())->position()};
    auto *buttonEvent{event.type() == MouseButtonEvent::type_? dynamic_cast<MouseButtonEvent *>(event()): nullptr};
    const auto wasCapturing{capture_.active()};
    const auto change{buttonEvent? capture_.track(*buttonEvent): MouseCapture::Change::none};
    Widget *widget{};
    bool own{true};
    if (wasCapturing and change != MouseCapture::Change::began) {
      // A mouse button is down: the motion and the release belong to the press, wherever the mouse is
      widget = grab_;
    } else {
      // Mouse input only concerns what is visible at the position: nothing covered
      // by another element, and nothing of another window
      Surface::Fragment *fragment{};
      try {
        fragment = host_.surface()->find(position);
      } catch (const std::logic_error &) { // no single topmost element at this position
        fragment = nullptr;
      }
      auto *element{fragment? fragment->element: nullptr};
      widget = dynamic_cast<Widget *>(element);
      own = element == &host_ or (widget and widget->root() == this);
      if (!own) {
        widget = nullptr;
      }
    }
    if (change == MouseCapture::Change::began) {
      grab_ = widget;
      if (own) {
        // Click to focus: the widget that was hit and the window it is in
        if (widget) {
          focus(widget);
        }
        host_.terminal()->focus(&host_);
      }
    }
    const auto handled{widget? widget->event(event): false};
    if (change == MouseCapture::Change::ended) {
      grab_ = nullptr;
    }
    return handled;
  }
  // Other input goes to the focused widget, unless it is completely covered
  if (focus_ and !focus_->fragments().empty()) {
    return focus_->event(event);
  }
  return false;
}

} // namespace jwezel::ui
