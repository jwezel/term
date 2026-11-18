#pragma once

#include "widget.hh"

#include <vector>

namespace jwezel::ui {

///
/// Popup that shows a list of items, one per row, with one of them highlighted
///
/// The popup is a plain widget: it only displays the items and keeps track of the
/// highlight and of what is scrolled into view. It leaves all interaction to the
/// widget that owns it (see `eventTarget`), which can therefore remove the
/// popup at any time. Only the first row of an item is shown, between the
/// glyphs that the UI style has for the items of a popup. The popup has the
/// colors of the role `popup`, and the highlighted item those of
/// `popup.highlight` (the default is reverse video) on top of that.
struct ListPopup: Widget {
  ///
  /// Create ListPopup
  ///
  /// @param      parent       The parent container, usually the root so that the popup
  ///                          is not subject to any layout
  /// @param      area         The area; as many rows as it has are visible at a time
  /// @param      items        The items
  /// @param      highlighted  Index of the item to highlight
  /// @param      owner        The widget the popup belongs to, which gets its mouse input
  /// @param      background   The background
  ListPopup(
    Container &parent,
    const Rectangle &area,
    const std::vector<Text> &items,
    Dim highlighted,
    Widget &owner,
    const Char &background = Space
  );

  ListPopup(const ListPopup &) = delete;

  ListPopup(ListPopup &&) = delete;

  auto operator=(const ListPopup &) -> ListPopup & = delete;

  auto operator=(ListPopup &&) -> ListPopup & = delete;

  ~ListPopup() override = default;

  [[nodiscard]] auto eventTarget() -> Widget * override {return &owner_;}

  ///
  /// Get the index of the highlighted item
  [[nodiscard]] auto highlighted() const -> Dim {return highlighted_;}

  ///
  /// Highlight an item
  ///
  /// The index is limited to the existing items, and the list scrolls to show the item.
  ///
  /// @param[in]  index  The index
  void highlight(Dim index);

  ///
  /// Get the item at a position
  ///
  /// @param[in]  position  A position relative to the root
  ///
  /// @return     The index of the item, or -1 if the position is not on an item
  [[nodiscard]] auto itemAt(const Vector &position) -> Dim;

  ///
  /// Get the number of rows that are visible at a time
  [[nodiscard]] auto rows() const -> Dim {return area().height();}

  protected:
  void render() override;

  private:
  std::vector<Text> items_;
  Dim highlighted_;
  Dim first_{0}; // index of the first visible item
  Widget &owner_;
};

} // namespace jwezel::ui
