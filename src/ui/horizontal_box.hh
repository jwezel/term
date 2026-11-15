#pragma once

#include "box.hh"

namespace jwezel::ui {

///
/// Box that lays out its children left to right
///
/// The layout is supplied in the constructor and fixed for the lifetime of
/// the box. Children stretch across the full height and share the width
/// according to their weights.
struct HorizontalBox: Box {
  explicit HorizontalBox(Container &parent, const Rectangle &area, const BoxLayout &layout = {}):
  Box{parent, area, layout, Orientation::horizontal}
  {}
};

} // namespace jwezel::ui
