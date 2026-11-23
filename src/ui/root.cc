#include "root.hh"
#include "widget.hh"

#include <term/keyboard.hh>
#include <term/surface.hh>
#include <term/term_interface.hh>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace jwezel::ui {

Root::Root(jwezel::Window &host, const ColorStyle *colors, const UiStyle *style):
Container{nullptr},
host_{host},
client_{&host.content(), Rectangle{Vector{0, 0}, host.area().size()}, host.background()},
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

void Root::enroll(Widget &widget) {
  chain_.push_back(&widget);
}

void Root::step(int direction) {
  const auto count{chain_.size()};
  if (count == 0) {
    return;
  }
  const auto current{std::ranges::find(chain_, focus_)};
  // From nowhere, the next one is the first and the previous one the last
  const auto start{
    current == chain_.end()?
      (direction > 0? count - 1: 0)
    :
      static_cast<std::size_t>(current - chain_.begin())
  };
  for (std::size_t distance = 1; distance <= count; ++distance) {
    const auto index{(start + (direction > 0? distance: count - distance)) % count};
    if (auto *candidate{chain_[index]}; candidate->tabbable()) {
      if (candidate != focus_) {
        focus(candidate);
      }
      return;
    }
  }
}

void Root::focusNext() {
  step(1);
}

void Root::focusPrevious() {
  step(-1);
}

void Root::tabOrder(Widget &first, Widget &second) {
  const auto from{std::ranges::find(chain_, &second)};
  if (std::ranges::find(chain_, &first) == chain_.end() or from == chain_.end()) {
    throw std::invalid_argument("A widget that is not in the window cannot be put in order");
  }
  if (&first == &second) {
    return;
  }
  chain_.erase(from);
  chain_.insert(std::ranges::find(chain_, &first) + 1, &second);
}

void Root::tabOrder(const std::vector<Widget *> &order) {
  for (std::size_t index = 1; index < order.size(); ++index) {
    tabOrder(*order[index - 1], *order[index]);
  }
}

void Root::forget(Widget &widget) {
  std::erase(chain_, &widget);
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
  // Ensure the host window is large enough for the child's area
  if (!child.layoutManaged()) {
    ensure(child.bounds());
    layout(); // resize the client to match the expanded host
  }
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
  client_.move(content() + contentOrigin()); // the client area of the host window is where the elements are clipped
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

auto Root::contentOrigin() const -> Vector {
  return Vector{0, 0};
}

void Root::ensure(const Rectangle &area) {
  host_.terminal()->expand(displayOrigin() + area.position2());
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

///
/// Whether any part of the widget shows on the display
///
/// A widget is covered where an element of its own window covers it, which its fragments already say,
/// and where another window covers the window. The widget is visible where a fragment of it, mapped to
/// the display, overlaps a visible fragment of the window.
///
auto Root::visible(const Widget &widget) const -> bool {
  // A host-placed widget is already on the surface of the host window, in display coordinates; a content widget is
  // mapped from the client to the display
  const auto offset{
    widget.placement() == WidgetPlacement::host?
      Vector{0, 0}
    :
      client().area().position() + host_.area().position()
  };
  for (const auto &widgetFragment: widget.fragments()) {
    const auto at{widgetFragment.area.position() + offset};
    const Rectangle shown{at, at + widgetFragment.area.size()};
    for (const auto &windowFragment: host_.fragments()) {
      if (windowFragment.area.intersects(shown)) {
        return true;
      }
    }
  }
  return false;
}

auto Root::handleEvent(const Event &event) -> bool {
  if (event.type() == MouseMoveEvent::type_ or event.type() == MouseButtonEvent::type_) {
    const auto position{event.as<MouseEvent>().position()};
    const auto *buttonEvent{event.get<MouseButtonEvent>()};
    const auto wasCapturing{capture_.active()};
    const auto change{buttonEvent? capture_.track(*buttonEvent): MouseCapture::Change::none};
    Widget *widget{};
    bool own{true};
    if (wasCapturing and change != MouseCapture::Change::began) {
      // A mouse button is down: the motion and the release belong to the press, wherever the mouse is
      widget = grab_;
    } else {
      // Mouse input only concerns what is visible at the position: nothing covered
      // by another element, and nothing of another window. A widget placed above the host window is on the
      // surface of the host window itself; the other widgets are in the client content. A click that finds no
      // widget is on the frame, on the empty client area, or on the underlay beneath the widgets: it is this
      // window's own, because the terminal routes it here only when the window is what shows there
      const auto *hostFragment{host_.surface()->find(position)};
      auto *hostElement{hostFragment? hostFragment->element: nullptr};
      if (auto *transient{dynamic_cast<Widget *>(hostElement)};
          transient and transient->placement() == WidgetPlacement::host and transient->root() == this) {
        widget = transient;
      } else {
        const auto clientPosition{position - host_.area().position() - client().area().position()};
        const auto *fragment{client().content().find(clientPosition)};
        auto *element{fragment? fragment->element: nullptr};
        widget = dynamic_cast<Widget *>(element);
        own = not widget or widget->root() == this;
        if (!own) {
          widget = nullptr;
        }
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
        if (target and target->clickable()) {
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
  auto handled{false};
  if (focus_ and visible(*focus_)) {
    handled = focus_->event(event);
  }
  if (!handled and event.type() == KeyEvent::type_) {
    handled = handleKey(event);
  }
  return handled;
}

auto Root::handleKey(const Event &event) -> bool {
  const auto key{event.as<KeyEvent>().key()};
  // What the focused widget offers, then what the window offers, then what is always there
  if (focus_ and focus_->triggerShortcut(key)) {
    return true;
  }
  if (triggerShortcut(key)) {
    return true;
  }
  if (key == Key::Tab) {
    focusNext();
    return true;
  }
  if (key == Key::ShiftTab) {
    focusPrevious();
    return true;
  }
  return false;
}

} // namespace jwezel::ui
