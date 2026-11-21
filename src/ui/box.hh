#pragma once

#include "container.hh"

#include <vector>

#include <term/geometry.hh>

namespace jwezel::ui {

/// How a box sizes a child along its main axis.
enum class LayoutSize: u1 {
  stretch, ///< The child gets a share of the room, according to its weight
  fit      ///< The child gets the size it asks for (`Element::preferredSize()`)
};

/// Where a box places a child across its cross axis.
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
/// Layout policy for a `Box`.
///
/// The layout is supplied to the box constructor and fixed for its lifetime. Per-child vectors are indexed by child
/// insertion order; entries beyond each vector's length use that property's default.
struct BoxLayout {
  std::vector<double> weights; ///< Relative main-axis share per stretching child; missing entries default to 1.
  std::vector<LayoutSize> sizes; ///< Main-axis sizing per child; missing entries default to `stretch`.
  std::vector<LayoutAlign> alignments; ///< Cross-axis alignment per child; missing entries default to `stretch`.
  int spacing = 0; ///< Empty cells between adjacent children along the main axis.
  int padding = 0; ///< Inset on all sides between the box bounds and its content area.
};

/// Direction in which a box arranges its children.
enum class Orientation {
  horizontal, ///< Left to right; width is the main axis.
  vertical    ///< Top to bottom; height is the main axis.
};

/// Container that arranges children along one axis.
///
/// Children are laid out in insertion order. Stretch children divide remaining main-axis space by weight; fit
/// children request their preferred size. If fit children exceed the available space, earlier children receive
/// space first. A child's preferred size also controls its cross-axis size when alignment is not `stretch`.
struct Box: Container {

  /// Create a box that arranges children in the requested orientation.
  /// @param parent Parent container; it must outlive the box.
  /// @param area The box area relative to its root, or `RectangleDefault` for parent-managed sizing.
  /// @param layout Per-child sizing/alignment plus spacing and padding.
  /// @param orientation Horizontal or vertical arrangement.
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

  /// Recompute the position and size of every child from the current bounds and layout policy.
  void layout();

  ///
  /// The size the box would like to have: its children, one after the other along the main axis
  [[nodiscard]] auto preferredSize() -> Vector override;

  /// Re-layout children after a child's preferred size changes, then notify the parent.
  void layoutChanged() override;

  /// Get the box's assigned area relative to its root.
  [[nodiscard]] auto bounds() -> Rectangle override {return area_;}

  /// Get the direction in which this box arranges its children.
  [[nodiscard]] auto orientation() const -> Orientation {return orientation_;}

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
