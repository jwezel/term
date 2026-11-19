#pragma once

#include "container.hh"

#include <vector>

#include <term/geometry.hh>

namespace jwezel::ui {

///
/// How a box sizes a child along its main axis
enum class LayoutSize: u1 {
  stretch, ///< The child gets a share of the room, according to its weight
  fit      ///< The child gets the size it asks for (`Element::preferredSize()`)
};

///
/// Where a box places a child across its main axis
///
/// A child that is not stretched across the cross axis gets its preferred size there, and is placed at the
/// beginning, in the middle or at the end of the room it has.
enum class LayoutAlign: u1 {
  stretch, ///< The child fills the cross axis (the default)
  begin,   ///< The preferred size, at the top or at the left
  center,  ///< The preferred size, in the middle
  end      ///< The preferred size, at the bottom or at the right
};

///
/// Layout description of a Box
///
/// The layout is supplied to the box constructor and fixed for the lifetime
/// of the box. All lists are in order of addition; children beyond the end of
/// a list get the default of it.
///
/// @param weights     Stretch factor per child; the default is 1
/// @param sizes       Whether a child stretches along the main axis or is as large as it prefers
/// @param alignments  Where a child is placed across the main axis
/// @param spacing     Empty space between children along the layout axis
/// @param padding     Inset of the content area on all sides
struct BoxLayout {
  std::vector<double> weights;
  std::vector<LayoutSize> sizes;
  std::vector<LayoutAlign> alignments;
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

  ///
  /// The size the box would like to have: its children, one after the other along the main axis
  [[nodiscard]] auto preferredSize() -> Vector override;

  void layoutChanged() override;

  [[nodiscard]] auto bounds() -> Rectangle override {return area_;}

  private:
  [[nodiscard]] auto weight(Dim index) const -> double;

  [[nodiscard]] auto sizing(Dim index) const -> LayoutSize;

  [[nodiscard]] auto alignment(Dim index) const -> LayoutAlign;

  Rectangle area_;
  BoxLayout layout_;
  Orientation orientation_;
  std::vector<Element *> ordered_; // weak references, in addition order
};

} // namespace jwezel::ui
