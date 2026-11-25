#pragma once

#include "box.hh"

namespace jwezel::ui {

///
/// Box that stacks its children top to bottom
///
/// Children take the full width of the box unless their cross-axis alignment asks for their own width and places
/// them at the beginning, center, or end. On the main axis, `BoxLayout::sizes` and `BoxLayout::weights` decide how
/// the height is shared: a fit child gets its preferred height, the stretching ones share what is left.
///
/// The layout is supplied in the constructor and fixed for the lifetime of the box. An area of `RectangleDefault`
/// leaves the area to the parent layout.
struct VerticalBox: Box {
  VerticalBox(Container &parent, const Rectangle &area, const BoxLayout &layout = {}):
  Box{parent, area, layout, Orientation::vertical}
  {}
};

} // namespace jwezel::ui
