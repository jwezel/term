#include <yoga/Yoga.h>
#include <algorithm>
#include <memory>

#include "fmt/core.h"
#include "fmt/format.h"
#include "geometry.hh"
#include "element.hh"
#include "text.hh"
#include "update.hh"

using namespace jwezel;
using namespace jwezel::ui;

Element::Element(Element *parent, Orientation orientation, const Rectangle &area, struct Element *window_):
layout_{YGConfigGetDefault()},
layoutNode_{YGNodeNewWithConfig(layout_)},
needsDrawing_{true},
needsRendering_{true},
orientation_(orientation),
parent_(parent)/*,
window_(window_? window_: window())*/
{
  constexpr std::array<YGFlexDirection, 2> j2ydt{YGFlexDirectionRow, YGFlexDirectionColumn}; // TODO: move to a better place

  YGNodeStyleSetFlexDirection(layoutNode_, j2ydt[orientation_]);
  YGNodeStyleSetFlex(layoutNode_, 1.0);
  if (area != RectangleDefault) {
    YGNodeStyleSetWidth(layoutNode_, area.width());
    YGNodeStyleSetHeight(layoutNode_, area.height());
  } else {
    YGNodeStyleSetWidthAuto(layoutNode_);
    YGNodeStyleSetHeightAuto(layoutNode_);
  }
  if (parent_) {
    parent_->add(this);
  }
}

Element *Element::window() const {
  return parent_? parent_->window(): 0;
}

Updates Element::updated() {
  fmt::print(stderr, "Updated element {}\n", fmt::ptr(this));
  Updates result;
  if (needsDrawing_) {
    fmt::print(stderr, "Calling draw on {}\n", fmt::ptr(this));
    result = draw();
    needsDrawing_ = false;
    needsRendering_ = true;
  }
  return result;
}

void Element::render() {
  fmt::print(stderr, "Render element {}\n", fmt::ptr(this));
  // Gather updates from element and its children
  const Updates updates{updated()};
  for (const auto &e: children_) {
    e->render();
  }
  needsRendering_ = false;
  window()->update(updates);
}

void Element::doLayout() {
  YGNodeCalculateLayout(layoutNode_, width(), height(), YGNodeStyleGetDirection(layoutNode_));
}

Rectangle Element::area() const {
  // Sadly Yoga doesn't give the absolut position of sub-elements, just their position in the parent
  auto left{YGNodeLayoutGetLeft(layoutNode_)};
  auto top{YGNodeLayoutGetTop(layoutNode_)};
  for (auto e{parent_}; e; e = e->parent_) {
    left += YGNodeLayoutGetLeft(e->layoutNode_);
    top += YGNodeLayoutGetTop(e->layoutNode_);
  }
  return Rectangle(left, top, left + YGNodeLayoutGetWidth(layoutNode_), top + YGNodeLayoutGetHeight(layoutNode_));
}

void Element::area(const Rectangle &area) {
  if (area != RectangleDefault) {
    // Set geometry
  }
}

Vector Element::size() const {
  return Vector{static_cast<Dim>(YGNodeLayoutGetWidth(layoutNode_)), static_cast<Dim>(YGNodeLayoutGetHeight(layoutNode_))};
}

Dim Element::width() const {
  return static_cast<Dim>(YGNodeLayoutGetWidth(layoutNode_));
}

Dim Element::height() const {
  return static_cast<Dim>(YGNodeLayoutGetHeight(layoutNode_));
}

int Element::add(Element *element, Element *before) {
  children_.reserve((children_.size() / 8 + 1) * 8);
  auto insertPos{
    before?
      find_if(children_.begin(), children_.end(), [before](const unique_ptr<Element> &e1) {return e1.get() == before;}):
      children_.end()
  };
  children_.insert(insertPos, unique_ptr<Element>(element));
  auto ygInsertPos{insertPos - children_.begin()};
  YGNodeInsertChild(layoutNode_, element->layoutNode_, ygInsertPos);
  return ygInsertPos;
}

int Element::add(Element *element) {
  return add(element, 0);
}
