#include <Yoga.h>

#include "YGStyle.h"

#include "container.hh"
#include "geometry.hh"
#include "element.hh"
#include "text.hh"
#include "update.hh"
#include "window.hh"

using namespace jwezel;
using namespace jwezel::ui;

using tl::optional;

Element::Element(Element *parent, Orientation orientation, const Rectangle &area, struct Window *window):
layout_{YGConfigGetDefault()},
layoutNode_{YGNodeNewWithConfig(layout_)},
needsDrawing_{true},
needsRendering_{true},
orientation_(orientation),
parent_(parent),
window_(window? window: (parent? parent->window_: 0))
{
  constexpr std::array<YGFlexDirection, 2> j2ydt{YGFlexDirectionRow, YGFlexDirectionColumn}; // TODO: move to a better place

  YGNodeStyleSetFlexDirection(layoutNode_, j2ydt[orientation_]);
  YGNodeSetContext(layoutNode_, this);
  if (parent_) {
    if (area != RectangleDefault) {
      // Set geometry
    }
    auto insertPos{parent_->add(this)};
    YGNodeInsertChild(dynamic_cast<Element *>(parent_)->layoutNode_, layoutNode_, insertPos);
  }
}

Updates Element::updated() {
  Updates result;
  // Draw if needed
  if (needsDrawing_) {
    result = draw();
    needsDrawing_ = false;
    needsRendering_ = true;
  }
  return result;
}

void Element::render() {
  // Send contents to window
  Updates updates{updated()};
  if (!updates.empty()) {
    window_->update(updates);
  }
  needsRendering_ = false;
}

void Element::doLayout() {
  YGNodeCalculateLayout(layoutNode_, width(), height(), YGNodeStyleGetDirection(layoutNode_));
}

Rectangle Element::area() const {
  return Rectangle(
    YGNodeLayoutGetLeft(layoutNode_),
    YGNodeLayoutGetTop(layoutNode_),
    YGNodeLayoutGetRight(layoutNode_),
    YGNodeLayoutGetBottom(layoutNode_)
  );
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
  auto ygInsertPos{add(element, before)};
  if (parent_) {
    Element &parentElement = *dynamic_cast<Element *>(parent_);
    YGNodeInsertChild(parentElement.layoutNode_, layoutNode_, ygInsertPos);
#ifndef NDEBUG
    if (parent_) {
      assert(YGNodeGetChildCount(parentElement.layoutNode_) == children_.size());
    }
#endif
  }
  return ygInsertPos;
}

int Element::add(Element *element) {
  return add(element, 0);
}
