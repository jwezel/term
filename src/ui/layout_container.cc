#include <algorithm>
#include <array>
#include <vector>

#include <range/v3/view/transform.hpp>
#include "range/v3/range/conversion.hpp"
#include "Utils.h"
#include <Yoga.h>

#include "element.hh"
#include "geometry.hh"
#include "layout_container.hh"
#include "element.hh"
#include "multi_container.hh"

using namespace jwezel::ui;

LayoutContainer::LayoutContainer(Container *parent, Orientation orientation, const Rectangle &area):
Element{parent, area},
orientation(orientation)
{
  constexpr std::array<YGFlexDirection, 2> j2ydt{YGFlexDirectionRow, YGFlexDirectionColumn}; // TODO: move to a better place

  YGNodeStyleSetFlexDirection(layoutNode, j2ydt[orientation]);
  YGNodeStyleSetJustifyContent(layoutNode, YGJustifyFlexEnd);
}

int LayoutContainer::add(Element *element, Element *before) {
  auto ygInsertPos{MultiContainer::add(element, before)};
  if (Container::parent) {
    Element &parentElement = *dynamic_cast<Element *>(Container::parent);
    YGNodeInsertChild(parentElement.layoutNode, layoutNode, ygInsertPos);
#ifndef NDEBUG
    if (Container::parent) {
      assert(YGNodeGetChildCount(parentElement.layoutNode) == children.size());
    }
#endif
  }
  return ygInsertPos;
}

int LayoutContainer::add(Element *element) {
  return add(element, 0);
}
