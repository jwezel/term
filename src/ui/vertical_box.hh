#pragma once

#include "box.hh"

namespace jwezel::ui {

///
/// Box that stacks its children top to bottom
///
/// The layout is supplied in the constructor and fixed for the lifetime of
/// the box. Children stretch across the full width and share the height
/// according to their weights.
struct VerticalBox: Box {
  explicit VerticalBox(Container &parent, const Rectangle &area, const BoxLayout &layout = {}):
  Box{parent, area, layout, Orientation::vertical}
  {}
};

} // namespace jwezel::ui
