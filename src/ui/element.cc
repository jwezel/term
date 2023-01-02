#include <yoga/Yoga.h> // NOTE: only works when placed on top 😱

#include "geometry.hh"
#include "element.hh"
#include "text.hh"
#include "update.hh"

#include <cstdio>
#include <iterator>
#include <algorithm>
#include <memory>

#include <fmt/core.h>
#include <fmt/format.h>

using namespace jwezel;
using namespace jwezel::ui;

Element::Element(Element *parent, Orientation orientation, const Vector &size):
layout_{YGConfigGetDefault()},
layoutNode_{YGNodeNewWithConfig(layout_)},
needsDrawing_{true},
needsRendering_{true},
orientation_(orientation),
parent_(parent)
{
  constexpr std::array<YGFlexDirection, 2> j2ydt{YGFlexDirectionRow, YGFlexDirectionColumn}; // TODO: move to a better place

  YGNodeStyleSetFlexDirection(layoutNode_, j2ydt[orientation_]);
  YGNodeStyleSetFlex(layoutNode_, 1.0);
  if (size.x != DimLow) {
    YGNodeStyleSetWidth(layoutNode_, size.x);
  } else {
    YGNodeStyleSetWidthAuto(layoutNode_);
  }
  if (size.y != DimLow) {
    YGNodeStyleSetHeight(layoutNode_, size.y);
  } else {
    YGNodeStyleSetHeightAuto(layoutNode_);
  }
  if (parent_) {
    parent_->add(this);
  }
}

const Element *Element::window() const {
  return parent_? parent_->window(): 0;
}

Updates Element::updated() {
  // fmt::print(stderr, "Updated element {}\n", fmt::ptr(this));
  Updates result;
  if (needsDrawing_) {
    // fmt::print(stderr, "Calling draw on {}\n", fmt::ptr(this));
    result = draw();
    needsDrawing_ = false;
    needsRendering_ = true;
  }
  return result;
}

Updates Element::render() {
  // fmt::print(stderr, "Render element {}\n", fmt::ptr(this));
  // Gather updates from element and its children
  Updates updates{updated()};
  for (const auto &e: children_) {
    auto subUpdates{e->render()};
    copy(subUpdates.begin(), subUpdates.end(), back_inserter(updates));
  }
  needsRendering_ = false;
  // fmt::print(stderr, "{} updates\n", updates.size());
  return updates;
  // const_cast<Element *>(window())->update(updates);
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
  auto insertPos{
    before?
      find_if(children_.begin(), children_.end(), [before](const unique_ptr<Element> &e1) {return e1.get() == before;})
    :
      children_.end()
  };
  children_.insert(insertPos, unique_ptr<Element>(element));
  auto ygInsertPos{insertPos - children_.begin()};
  children_.insert(insertPos, element);
  YGNodeInsertChild(layoutNode_, element->layoutNode_, ygInsertPos);
  return ygInsertPos;
}

int Element::add(Element *element) {
  return add(element, 0);
}

void Element::setMinimumWidth(Dim width) {
    YGNodeStyleSetMinWidth(layoutNode_, width);
}

void Element::setMaximumWidth(Dim width) {
    YGNodeStyleSetMaxWidth(layoutNode_, width);
}

void Element::setMinimumHeight(Dim height) {
    YGNodeStyleSetMinHeight(layoutNode_, height);
}

void Element::setMaximumHeight(Dim height) {
    YGNodeStyleSetMaxHeight(layoutNode_, height);
}

void Element::setPadding(const Rectangle &padding) {
  if (padding.x1 != DimLow) {
    YGNodeStyleSetPadding(layoutNode_, YGEdgeLeft, padding.x1);
  }
  if (padding.x2 != DimLow) {
    YGNodeStyleSetPadding(layoutNode_, YGEdgeRight, padding.x2);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetPadding(layoutNode_, YGEdgeTop, padding.y1);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetPadding(layoutNode_, YGEdgeBottom, padding.y1);
  }
}

void Element::setBorder(const Rectangle &padding) {
  if (padding.x1 != DimLow) {
    YGNodeStyleSetBorder(layoutNode_, YGEdgeLeft, padding.x1);
  }
  if (padding.x2 != DimLow) {
    YGNodeStyleSetBorder(layoutNode_, YGEdgeRight, padding.x2);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetBorder(layoutNode_, YGEdgeTop, padding.y1);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetBorder(layoutNode_, YGEdgeBottom, padding.y1);
  }
}

void Element::setMargin(const Rectangle &padding) {
  if (padding.x1 != DimLow) {
    YGNodeStyleSetMargin(layoutNode_, YGEdgeLeft, padding.x1);
  }
  if (padding.x2 != DimLow) {
    YGNodeStyleSetMargin(layoutNode_, YGEdgeRight, padding.x2);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetMargin(layoutNode_, YGEdgeTop, padding.y1);
  }
  if (padding.y1 != DimLow) {
    YGNodeStyleSetMargin(layoutNode_, YGEdgeBottom, padding.y1);
  }
}
