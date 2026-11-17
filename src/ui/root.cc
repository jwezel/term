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

Root::Root(jwezel::Window &host, const ColorStyle *colors, const UiStyle *style):
Container{nullptr},
host_{host},
colors_{colors},
style_{style}
{
  host_.inputHandler(this);
}

Root::~Root() {
  host_.inputHandler(nullptr);
}

auto Root::active() const -> bool {
  return host_.terminal()->focus() == &host_;
}

auto Root::colors() const -> const ColorStyle & {
  return colors_? *colors_: defaultColorStyle();
}

void Root::colors(const ColorStyle *colors) {
  colors_ = colors;
  restyle();
}

auto Root::style() const -> const UiStyle & {
  return style_? *style_: defaultUiStyle();
}

void Root::style(const UiStyle *style) {
  style_ = style;
  restyle();
}

namespace {

void restyleAll(const Container &container) {
  for (auto *child: container.children()) {
    if (auto *widget{dynamic_cast<Widget *>(child)}; widget) {
      widget->restyle();
    }
    if (const auto *inner{dynamic_cast<Container *>(child)}; inner) {
      restyleAll(*inner);
    }
  }
}

} // namespace

void Root::restyle() {
  restyleAll(*this);
}

void Root::focus(Widget *widget) {
  auto *previous{focus_};
  focus_ = widget;
  if (previous and previous != widget) {
    previous->focusChanged(false);
  }
  updateCaret();
  if (widget and previous != widget) {
    widget->focusChanged(true);
  }
}

void Root::updateCaret() {
  host_.terminal()->caret(host_, focus_, focus_? focus_->caret(): std::nullopt);
}

void Root::forget(Widget &widget) {
  if (grab_ == &widget) {
    grab_ = nullptr;
  }
  if (focus_ == &widget) {
    focus_ = nullptr; // no notification: the widget is on its way out
    updateCaret();
  }
}

auto Root::add(Element &child) -> Container & {
  Container::add(child);
  if (child.layoutManaged()) {
    if (const auto area{content()}; area.width() > 0 and area.height() > 0) {
      child.move(area);
    }
  }
  return *this;
}

void Root::move(const Rectangle &area) {
  host_.jwezel::Window::move(area); // the elements follow in handleGeometry
}

void Root::layout() {
  const auto area{content()};
  for (auto *child: children()) {
    if (!child->layoutManaged()) {
      child->move(child->bounds());
    } else if (area.width() > 0 and area.height() > 0) {
      child->move(area);
    }
  }
  updateCaret();
}

void Root::handleGeometry(const Rectangle &/*previous*/) {
  layout();
}

void Root::handleFocus() {
  updateCaret();
  if (focus_) {
    focus_->focusChanged(true);
  }
}

void Root::handleBlur() {
  if (focus_) {
    focus_->focusChanged(false);
  }
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
      const auto *fragment{host_.surface()->find(position)};
      auto *element{fragment? fragment->element: nullptr};
      widget = dynamic_cast<Widget *>(element);
      own = element == &host_ or (widget and widget->root() == this);
      if (!own) {
        widget = nullptr;
      }
    }
    auto *target{widget? widget->eventTarget(): nullptr};
    if (change == MouseCapture::Change::began) {
      grab_ = widget;
      if (own) {
        // Click to focus: the widget that was hit and the window it is in
        if (focus_ and target != focus_) {
          focus_->pressedElsewhere();
        }
        if (target) {
          focus(target);
        }
        host_.terminal()->focus(&host_);
      }
    }
    const auto handled{target? target->event(event): false};
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
