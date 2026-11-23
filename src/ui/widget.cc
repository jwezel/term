#include "container.hh"
#include "element.hh"
#include "root.hh"
#include "widget.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

#include <exception>

namespace jwezel::ui {

// The background of the widget, which is the one of the window unless it has one of its own
auto resolvedBackground(const Char &background, Container &parent) -> Char {
  return background == InheritBackground? parent.root()->host().background(): background;
}

Widget::Widget(const Rectangle &area, const Char &background, Container &parent, WidgetPlacement placement):
TextElement{
  placement == WidgetPlacement::host? parent.root()->host().surface(): &parent.root()->client().content(),
  placement == WidgetPlacement::host? area + parent.root()->displayOrigin(): area,
  resolvedBackground(background, parent)
},
ui::Element{&parent},
bounds_{area},
placement_{placement}
{
  layoutManaged(area == RectangleDefault);
  parent.add(*this);
  auto *root{parent.root()};
  root->enroll(*this);
  if (!root->focus()) {
    root->focus(this);
  }
}

Widget::~Widget() {
  try {
    root()->forget(*this);
  } catch (const std::exception &) { // no root to tell
  }
}

void Widget::move(const Rectangle &area) {
  auto *root{this->root()};
  root->ensure(area); // the host window must be large enough to show the area
  TextElement::move(placement_ == WidgetPlacement::host? area + root->displayOrigin(): area);
}

void Widget::horizontalAlignment(ContentAlignment alignment) {
  if (horizontalAlignment_ == alignment) {
    return;
  }
  horizontalAlignment_ = alignment;
  render();
}

void Widget::verticalAlignment(ContentAlignment alignment) {
  if (verticalAlignment_ == alignment) {
    return;
  }
  verticalAlignment_ = alignment;
  render();
}

void Widget::contentAlignment(ContentAlignment horizontal, ContentAlignment vertical) {
  if (horizontalAlignment_ == horizontal and verticalAlignment_ == vertical) {
    return;
  }
  horizontalAlignment_ = horizontal;
  verticalAlignment_ = vertical;
  render();
}

auto Widget::inputWindow() -> jwezel::Window * {
  return &root()->host();
}

auto Widget::tabbable() -> bool {
  return (focusPolicy_ == FocusPolicy::tab or focusPolicy_ == FocusPolicy::strong) and root()->visible(*this);
}

void Widget::focusPolicy(FocusPolicy policy) {
  focusPolicy_ = policy;
  auto *root{this->root()};
  if (policy == FocusPolicy::none and root->focus() == this) {
    root->focus(nullptr);
  } else if ((policy == FocusPolicy::tab or policy == FocusPolicy::strong) and !root->focus()) {
    root->focus(this);
  }
}

auto Widget::focused() -> bool {
  auto *root{this->root()};
  return root->focus() == this and root->active();
}

auto Widget::colors() -> const ColorStyle & {
  return root()->colors();
}

auto Widget::uiStyle() -> const UiStyle & {
  return root()->style();
}

auto Widget::widgetState(bool inUse) -> WidgetState {
  if (inUse) {
    return WidgetState::active;
  }
  return focused()? WidgetState::focus: WidgetState::normal;
}

auto Widget::canvas(std::string_view role) -> Text {
  Text result{background(), area().size()};
  // The cells replace what the widget showed before when they are written, whatever attributes that had
  result.setAttr(colors()[role], RectangleMax, AttributeMode::replace);
  return result;
}

void Widget::overlay(Text &canvas, const Text &content, const Vector &position) {
  canvas.patch(content, position, AttributeMode::merge, AttributeMode::default_, AttributeMode::replace);
}

auto Widget::contentPosition(const Vector &contentSize) const -> Vector {
  const auto offset = [](Dim available, Dim content, ContentAlignment alignment) -> Dim {
    const auto difference{static_cast<int>(available) - content};
    switch (alignment) {
      case ContentAlignment::center:
      return toDim(difference / 2);
      case ContentAlignment::end:
      return toDim(difference);
      case ContentAlignment::begin:
      default:
      return 0;
    }
  };
  const auto size{area().size()};
  return Vector{
    offset(size.x(), contentSize.x(), horizontalAlignment_),
    offset(size.y(), contentSize.y(), verticalAlignment_)
  };
}

void Widget::layoutChanged() {
  for (auto *dependent: layoutDependents_) {
    if (dependent and dependent->parent()) {
      dependent->parent()->layoutChanged();
    }
  }
  ui::Element::layoutChanged();
}

auto Widget::moveEvent(const Rectangle &area) -> bool {
  TextElement::moveEvent(area);
  bounds_ = placement_ == WidgetPlacement::host? area - root()->displayOrigin(): area;
  render();
  return true;
}

} // namespace jwezel::ui
