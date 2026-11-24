#pragma once

#include "list_source.hh"
#include "selection.hh"
#include "widget.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

namespace jwezel::ui {

///
/// A scrolling view of a list source
///
/// The view owns the source as a value, fetches only the rows that are visible, and keeps the current item and the
/// selection. It is the body that list boxes, drop-down popups, and menus share; those add their own interaction and
/// colors through the protected hooks.
///
/// The source can be lazy: only `size()` and the visible items are asked for. The preferred size scans at most
/// `preferredScanLimit()` items (256 by default), so a source with a million items does not cost a million calls to
/// lay out a widget.
///
/// @tparam     Index  The index type, which is the one the source reports from `size()`
template<std::signed_integral Index = ListIndex>
struct ListView: Widget {
  ///
  /// Create ListView
  ///
  /// @param      parent      The parent container
  /// @param      area        The area; as many rows as it has are visible at a time
  /// @param      source      The items
  /// @param      currentIndex  Index of the current item
  /// @param      background  The background
  ListView(
    Container &parent,
    const Rectangle &area,
    ListSource<Index> source,
    Index currentIndex = Index{0},
    const Char &background = InheritBackground
  ):
  Widget{area, background, parent},
  source_{std::move(source)}
  {
    refreshData();
    current(currentIndex);
  }

  ListView(const ListView &) = delete;

  ListView(ListView &&) = delete;

  auto operator=(const ListView &) -> ListView & = delete;

  auto operator=(ListView &&) -> ListView & = delete;

  ~ListView() override = default;

  ///
  /// Get the source
  [[nodiscard]] auto source() const -> const ListSource<Index> & {return source_;}

  ///
  /// Replace the source; the current item and the selection are limited to the new items
  void source(ListSource<Index> source) {
    source_ = std::move(source);
    refresh();
  }

  ///
  /// Get the number of items, as of the last `refresh`
  [[nodiscard]] auto size() const -> Index {return count_;}

  ///
  /// Get an item from the source
  ///
  /// The index must be in `[0, size())`.
  [[nodiscard]] auto item(Index index) const -> Text {return source_.item(index);}

  ///
  /// Get an item from the source; the same as `item`
  [[nodiscard]] auto operator[](Index index) const -> Text {return item(index);}

  ///
  /// Get the current item
  ///
  /// @return     The index, or -1 if there are no items
  [[nodiscard]] auto current() const -> Index {return current_;}

  ///
  /// Make an item the current one
  ///
  /// The index is limited to the existing items and the view scrolls to show the item.
  ///
  /// @param[in]  index  The index
  void current(Index index) {
    if (count_ <= 0) {
      current_ = static_cast<Index>(-1);
      first_ = Index{0};
    } else {
      current_ = std::clamp(index, Index{0}, last());
      scrollToCurrent();
    }
    render();
  }

  ///
  /// Get the index of the first visible item
  [[nodiscard]] auto first() const -> Index {return first_;}

  ///
  /// Get the number of rows that are visible at a time
  [[nodiscard]] auto rows() const -> Dim {return area().height();}

  ///
  /// Get the item at a position
  ///
  /// @param[in]  position  A position relative to the root
  ///
  /// @return     The index of the item, or -1 if the position is not on an item
  [[nodiscard]] auto itemAt(const Vector &position) -> Index {
    const auto bounds{this->bounds()};
    const Rectangle point{position, position + Vector{1, 1}};
    if (not (bounds & point)) {
      return static_cast<Index>(-1);
    }
    const auto origin{contentPosition(Vector{bounds.width(), visibleRows()})};
    const auto row{position.y() - bounds.y1() - origin.y()};
    if (row < 0 or row >= visibleRows()) {
      return static_cast<Index>(-1);
    }
    const auto index{static_cast<i8>(first_) + row};
    return index >= 0 and index < count_? static_cast<Index>(index): static_cast<Index>(-1);
  }

  ///
  /// Get the selection
  [[nodiscard]] auto selection() const -> const Selection<Index> & {return selection_;}

  ///
  /// Whether an item is selected
  [[nodiscard]] auto selected(Index index) const -> bool {return selection_.contains(index);}

  ///
  /// Select only one item, or none for a negative index
  ///
  /// The selection is changed without telling anyone; this is the programmatic counterpart of what the user does.
  void select(Index index) {
    selection_.clear();
    if (index >= 0 and index < count_) {
      selection_.add(index);
    }
    render();
  }

  ///
  /// Add an item to the selection
  ///
  /// @return     Whether the item was not selected before
  auto addSelection(Index index) -> bool {
    if (index < 0 or index >= count_) {
      return false;
    }
    const auto added{selection_.add(index)};
    if (added) {
      render();
    }
    return added;
  }

  ///
  /// Remove an item from the selection
  ///
  /// @return     Whether the item was selected before
  auto removeSelection(Index index) -> bool {
    const auto removed{selection_.remove(index)};
    if (removed) {
      render();
    }
    return removed;
  }

  ///
  /// Select an item if it is not selected, and deselect it otherwise
  ///
  /// @return     Whether the item is selected now
  auto toggle(Index index) -> bool {
    if (index < 0 or index >= count_) {
      return false;
    }
    const auto result{selection_.toggle(index)};
    render();
    return result;
  }

  ///
  /// Deselect everything
  void clearSelection() {
    selection_.clear();
    render();
  }

  ///
  /// Select or deselect a range of items, both ends included
  ///
  /// @param[in]  from      The first index
  /// @param[in]  to        The last index
  /// @param[in]  selected  Whether to select or deselect
  void selectRange(Index from, Index to, bool selected = true) {
    if (from > to) {
      std::swap(from, to);
    }
    from = std::max(from, Index{0});
    if (count_ <= 0 or from > last()) {
      return;
    }
    to = std::min(to, last());
    for (auto index{from};; ++index) {
      if (selected) {
        selection_.add(index);
      } else {
        selection_.remove(index);
      }
      if (index == to) {
        break;
      }
    }
    render();
  }

  ///
  /// Scroll the viewport without moving the current item
  ///
  /// This is what the mouse wheel does: the current item can leave the view, and the next move of the current item
  /// brings it back.
  ///
  /// @param[in]  rows  The number of rows to scroll, negative to scroll up
  void scroll(Dim rows) {
    if (count_ <= 0) {
      return;
    }
    const auto visible{static_cast<i8>(std::max(Dim{1}, this->rows()))};
    const auto maximum{std::max<i8>(0, static_cast<i8>(count_) - visible)};
    const auto target{std::clamp(static_cast<i8>(first_) + rows, i8{0}, maximum)};
    first_ = static_cast<Index>(target);
    render();
  }

  ///
  /// Get the anchor of a range selection
  [[nodiscard]] auto anchor() const -> Index {return anchor_;}

  ///
  /// Set the anchor of a range selection
  void anchor(Index index) {anchor_ = index;}

  ///
  /// Read the source again after it has changed
  ///
  /// The current item and the selection are limited to the new items, and items that are gone are no longer
  /// selected. Nothing is emitted: a change of the data is not a choice of the user.
  void refresh() {
    refreshData();
    layoutChanged();
  }

  ///
  /// Get the maximum number of rows the preferred size asks for
  [[nodiscard]] auto maxRows() const -> Dim {return maxRows_;}

  ///
  /// Set the maximum number of rows the preferred size asks for
  void maxRows(Dim rows) {
    maxRows_ = std::max(Dim{1}, rows);
    layoutChanged();
  }

  ///
  /// Get how many items the preferred size scans for the width
  [[nodiscard]] auto preferredScanLimit() const -> std::size_t {return preferredScanLimit_;}

  ///
  /// Set how many items the preferred size scans for the width
  void preferredScanLimit(std::size_t limit) {
    preferredScanLimit_ = limit;
    layoutChanged();
  }

  ///
  /// Whether an item is enabled
  ///
  /// A disabled item is drawn in the disabled role and interaction skips it. All items are enabled unless a
  /// predicate is set with `itemEnabled`.
  [[nodiscard]] auto itemEnabled(Index index) const -> bool {
    return itemEnabled_? itemEnabled_(index): true;
  }

  ///
  /// Set which items are enabled
  ///
  /// @param[in]  predicate  The predicate, or an empty function for all items
  void itemEnabled(std::function<bool(Index)> predicate) {
    itemEnabled_ = std::move(predicate);
    render();
  }

  ///
  /// The width of the widest item in the scanned part of the source, and at most `maxRows` rows
  [[nodiscard]] auto preferredSize() -> Vector override {
    const auto &glyphs{itemGlyphs()};
    const auto padding{toDim(Text{glyphs.left}.width() + Text{glyphs.right}.width())};
    const auto markerWidth{toDim(Text{marker()}.width())};
    const auto extra{std::max(Dim{0}, toDim(markerWidth - Text{glyphs.left}.width()))};
    auto width{Dim{0}};
    const auto scan{std::min(static_cast<std::size_t>(std::max(count_, Index{0})), preferredScanLimit_)};
    for (std::size_t position = 0; position < scan; ++position) {
      width = std::max(width, item(static_cast<Index>(position)).width());
    }
    auto height{Dim{0}};
    if (count_ > 0) {
      height = count_ > maxRows_? maxRows_: static_cast<Dim>(count_);
    }
    return Vector{toDim(width + padding + extra), height};
  }

  protected:
  void render() override {
    const auto size{area().size()};
    if (size.x() <= 0 or size.y() <= 0) {
      return;
    }
    const auto &glyphs{itemGlyphs()};
    const Text left{glyphs.left}, right{glyphs.right};
    const Text mark{marker()};
    const auto contentStart{std::max(left.width(), mark.width())};
    const auto origin{contentPosition(Vector{size.x(), visibleRows()})};
    const auto padding{toDim(contentStart + right.width())};
    const auto textRoom{std::max(Dim{0}, toDim(size.x() - padding))};
    auto visible{canvas(backgroundRole())};
    for (Dim row = 0; row < visibleRows(); ++row) {
      const auto screenRow{toDim(origin.y() + row)};
      const auto wideIndex{static_cast<i8>(first_) + row};
      const auto index{static_cast<Index>(wideIndex)};
      const auto text{item(index)};
      const auto difference{static_cast<int>(textRoom) - text.width()};
      auto textOffset{Dim{0}};
      switch (horizontalAlignment()) {
        case ContentAlignment::center:
        textOffset = toDim(difference / 2);
        break;
        case ContentAlignment::end:
        textOffset = toDim(difference);
        break;
        case ContentAlignment::begin:
        default:
        break;
      }
      const auto clippedStart{std::max(Dim{0}, toDim(-std::min(0, static_cast<int>(textOffset))))};
      const auto shownWidth{std::max(Dim{0}, std::min(toDim(text.width() - clippedStart), textRoom))};
      overlay(visible, left, Vector{origin.x(), screenRow});
      overlay(visible, right, Vector{toDim(size.x() - right.width()), screenRow});
      if (text.height() > 0 and shownWidth > 0) {
        overlay(
          visible,
          text[Rectangle{clippedStart, 0, toDim(clippedStart + shownWidth), 1}],
          Vector{toDim(origin.x() + contentStart + std::max(Dim{0}, textOffset)), screenRow}
        );
      }
      const Rectangle rowArea{0, screenRow, size.x(), toDim(screenRow + 1)};
      if (not itemEnabled(index)) {
        visible.setAttr(colors()[disabledRole()], rowArea, AttributeMode::replace);
      }
      // Selection must remain visible even when the row is also current or disabled.
      if (selected(index)) {
        visible.setAttr(
          colors()[index == current_? currentSelectedRole(): selectedRole()],
          rowArea,
          AttributeMode::replace
        );
      } else if (index == current_) {
        visible.setAttr(colors()[currentRole()], rowArea, AttributeMode::replace);
      }
      if (index == current_ and markerVisible() and mark.width() > 0) {
        overlay(visible, mark, Vector{origin.x(), screenRow});
      }
    }
    write(Vector{0, 0}, visible);
  }

  ///
  /// The glyphs around an item
  [[nodiscard]] virtual auto itemGlyphs() -> const Brackets & {return uiStyle().list;}

  ///
  /// The color role of the view and of an idle item
  [[nodiscard]] virtual auto backgroundRole() -> std::string_view {
    return focused()? listFocusRole: listRole;
  }

  ///
  /// The color role of the current item
  [[nodiscard]] virtual auto currentRole() -> std::string_view {return listCurrentRole;}

  ///
  /// The color role of a selected item
  [[nodiscard]] virtual auto selectedRole() -> std::string_view {return listSelectedRole;}

  ///
  /// The color role of the current item when it is also selected
  [[nodiscard]] virtual auto currentSelectedRole() -> std::string_view {return listCurrentSelectedRole;}

  ///
  /// The color role of a disabled item
  [[nodiscard]] virtual auto disabledRole() -> std::string_view {return listDisabledRole;}

  ///
  /// The marker shown before the current item, or empty for none
  [[nodiscard]] virtual auto marker() -> std::string_view {return {};}

  ///
  /// Whether the marker is shown; it may still reserve space when hidden
  [[nodiscard]] virtual auto markerVisible() -> bool {return not marker().empty();}

  private:
  [[nodiscard]] auto visibleRows() const -> Dim {
    const auto size{area().size()};
    const auto remaining{
      std::max<std::int64_t>(0, static_cast<std::int64_t>(count_) - static_cast<std::int64_t>(first_))
    };
    return static_cast<Dim>(std::min<std::int64_t>(size.y(), remaining));
  }

  void refreshData() {
    count_ = source_.size();
    if (count_ < 0) {
      count_ = Index{0};
    }
    if (count_ <= 0) {
      current_ = static_cast<Index>(-1);
      first_ = Index{0};
      anchor_ = static_cast<Index>(-1);
      selection_.clear();
    } else {
      current_ = std::clamp(current_, Index{0}, last());
      first_ = std::clamp(first_, Index{0}, last());
      anchor_ = std::clamp(anchor_, Index{0}, last());
      selection_.truncate(count_);
      scrollToCurrent();
    }
    render();
  }
  [[nodiscard]] auto last() const -> Index {return static_cast<Index>(count_ - 1);}

  void scrollToCurrent() {
    if (current_ < 0) {
      first_ = Index{0};
      return;
    }
    const auto visible{static_cast<i8>(std::max(Dim{1}, rows()))};
    if (current_ < first_) {
      first_ = current_;
    }
    const auto end{static_cast<i8>(first_) + visible};
    if (current_ >= end) {
      first_ = static_cast<Index>(static_cast<i8>(current_) - visible + 1);
    }
    first_ = std::max(first_, Index{0});
  }

  ListSource<Index> source_;
  Index count_{0};
  Index current_{0};
  Index first_{0};
  Index anchor_{0};
  Selection<Index> selection_;
  Dim maxRows_{8};
  std::size_t preferredScanLimit_{256};
  std::function<bool(Index)> itemEnabled_;
};

} // namespace jwezel::ui
