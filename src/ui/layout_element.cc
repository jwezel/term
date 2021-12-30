#include <Yoga.h>

#include "YGStyle.h"
#include "geometry.hh"
#include "layout_element.hh"

using namespace jwezel;
using namespace jwezel::ui;

LayoutElement::LayoutElement(Container *parent, const Rectangle &area):
Element{parent},
layout{YGConfigGetDefault()},
layoutNode{YGNodeNewWithConfig(layout)}
{
  // YGConfigSetPointScaleFactor(layout, 1.);
  YGNodeSetContext(layoutNode, this);
  if (parent) {
    if (area != RectangleDefault) {
      // Set geometry
    }
    auto insertPos{parent->add(this)};
    YGNodeInsertChild(dynamic_cast<LayoutElement *>(parent)->layoutNode, layoutNode, insertPos);
  }
}

void LayoutElement::doLayout() {
  YGNodeCalculateLayout(layoutNode, width(), height(), YGNodeStyleGetDirection(layoutNode));
}

Rectangle LayoutElement::area() const {
  return Rectangle(
    YGNodeLayoutGetLeft(layoutNode),
    YGNodeLayoutGetTop(layoutNode),
    YGNodeLayoutGetRight(layoutNode),
    YGNodeLayoutGetBottom(layoutNode)
  );
}

void LayoutElement::area(const Rectangle &area) {
  if (area != RectangleDefault) {
    // Set geometry
  }
}

Vector LayoutElement::size() const {
  return Vector{static_cast<Dim>(YGNodeLayoutGetWidth(layoutNode)), static_cast<Dim>(YGNodeLayoutGetHeight(layoutNode))};
}

Dim LayoutElement::width() const {
  return static_cast<Dim>(YGNodeLayoutGetWidth(layoutNode));
}

Dim LayoutElement::height() const {
  return static_cast<Dim>(YGNodeLayoutGetHeight(layoutNode));
}
