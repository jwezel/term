#pragma once

#include "container.hh"

#include <vector>

#include <term/geometry.hh>

namespace jwezel::ui {

///
/// Layout description of a Box
///
/// The layout is supplied to the box constructor and fixed for the lifetime
/// of the box.
///
/// @param weights  Stretch factor per child, in order of addition; children
///                 beyond the end of the list get a weight of 1
/// @param spacing  Empty space between children along the layout axis
/// @param padding  Inset of the content area on all sides
struct BoxLayout {
  std::vector<double> weights;
  int spacing = 0;
  int padding = 0;
};

enum class Orientation {horizontal, vertical};

///
/// Container that arranges its children along one axis
///
/// Children are positioned as they are added, and repositioned when the box
/// itself is moved. Each child stretches across the full cross axis and gets
/// a slice of the main axis proportional to its weight; rounding is
/// accumulated so the slices always fill the box exactly.
struct Box: Container {

  Box(Container &parent, const Rectangle &area, const BoxLayout &layout, Orientation orientation);

  Box(const Box &) = delete;

  Box(Box &&) = delete;

  auto operator=(const Box &) -> Box & = delete;

  auto operator=(Box &&) -> Box & = delete;

  ~Box() override = default;

  auto add(Element &child) -> Container & override;

  ///
  /// Remove a child and lay out the remaining children again
  void remove(Element &child) override;

  ///
  /// Move box to a new area and lay out all children again
  ///
  /// @param[in]  area  The new area
  void move(const Rectangle &area) override;

  ///
  /// Recompute the position of all children
  void layout();

  [[nodiscard]] auto bounds() -> Rectangle override {return area_;}

  private:
  [[nodiscard]] auto weight(Dim index) const -> double;

  Rectangle area_;
  BoxLayout layout_;
  Orientation orientation_;
  std::vector<Element *> ordered_; // weak references, in addition order
};

} // namespace jwezel::ui
