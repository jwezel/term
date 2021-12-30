#include <algorithm>

#include <array>
#include <range/v3/view/transform.hpp>

#include <Yoga.h>
#include <vector>
#include "Utils.h"
#include "element.hh"
#include "geometry.hh"
#include "layout_container.hh"
#include "layout_element.hh"
#include "multi_container.hh"
#include "range/v3/range/conversion.hpp"

using namespace jwezel::ui;

LayoutContainer::LayoutContainer(Container *parent, Orientation orientation, const Rectangle &area):
LayoutElement{parent, area},
orientation(orientation)
{
  constexpr std::array<YGFlexDirection, 2> j2ydt{YGFlexDirectionRow, YGFlexDirectionColumn}; // TODO: move somewhere else

  YGNodeStyleSetFlexDirection(layoutNode, j2ydt[orientation]);
  YGNodeStyleSetJustifyContent(layoutNode, YGJustifyFlexEnd);
}

int LayoutContainer::add(Element *element, Element *before) {
  auto ygInsertPos{MultiContainer::add(element, before)};
  if (Container::parent) {
    LayoutElement &parentElement = *dynamic_cast<LayoutElement *>(Container::parent);
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
