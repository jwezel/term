#include "container.hh"
#include "element.hh"
#include "root.hh"
#include "widget.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

#include <exception>

namespace jwezel::ui {

namespace {

// Area on the display; an area that is left to the layout stays unassigned
auto displayArea(const Rectangle &area, Container &parent) -> Rectangle {
  return area == RectangleDefault? area: area + parent.root()->origin();
}

} // namespace

Widget::Widget(const Rectangle &area, const Char &background, Container &parent):
TextElement{
  parent.root()->host().surface(),
  displayArea(area, parent),
  background
},
ui::Element{&parent},
bounds_{area}
{
  layoutManaged(area == RectangleDefault);
  parent.add(*this);
  if (auto *root{parent.root()}; !root->focus()) {
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
  TextElement::move(area + root()->origin());
}

auto Widget::inputWindow() -> jwezel::Window * {
  return &root()->host();
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

auto Widget::moveEvent(const Rectangle &area) -> bool {
  TextElement::moveEvent(area);
  bounds_ = area - root()->origin();
  render();
  return true;
}

} // namespace jwezel::ui
