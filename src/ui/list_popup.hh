#pragma once

#include "list_view.hh"
#include "widget.hh"

#include <string_view>
#include <utility>

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
///
/// The items come from a `ListSource`, so the popup does not copy them: a drop-down that changes its items first
/// closes the popup, and a source may be lazy.
///
/// @tparam     Index  The index type
template<std::signed_integral Index = ListIndex>
struct ListPopup: ListView<Index> {
  ///
  /// Create ListPopup
  ///
  /// @param      parent       The parent container, usually the root so that the popup
  ///                          is not subject to any layout
  /// @param      area         The area; as many rows as it has are visible at a time
  /// @param      source       The items
  /// @param      highlighted  Index of the item to highlight
  /// @param      owner        The widget the popup belongs to, which gets its mouse input
  /// @param      background   The background
  ListPopup(
    Container &parent,
    const Rectangle &area,
    ListSource<Index> source,
    Index highlighted,
    Widget &owner,
    const Char &background = InheritBackground
  ):
  ListView<Index>{parent, area, std::move(source), highlighted, background},
  owner_{owner}
  {
    this->focusPolicy(FocusPolicy::none); // the widget it belongs to has the focus
    this->render();
  }

  ListPopup(const ListPopup &) = delete;

  ListPopup(ListPopup &&) = delete;

  auto operator=(const ListPopup &) -> ListPopup & = delete;

  auto operator=(ListPopup &&) -> ListPopup & = delete;

  ~ListPopup() override = default;

  [[nodiscard]] auto eventTarget() -> Widget * override {return &owner_;}

  ///
  /// Get the index of the highlighted item
  [[nodiscard]] auto highlighted() const -> Index {return this->current();}

  ///
  /// Highlight an item
  ///
  /// The index is limited to the existing items, and the list scrolls to show the item.
  ///
  /// @param[in]  index  The index
  void highlight(Index index) {this->current(index);}

  protected:
  [[nodiscard]] auto itemGlyphs() -> const Brackets & override {return this->uiStyle().popup;}

  [[nodiscard]] auto backgroundRole() -> std::string_view override {return popupRole;}

  [[nodiscard]] auto currentRole() -> std::string_view override {return popupHighlightRole;}

  [[nodiscard]] auto selectedRole() -> std::string_view override {return popupHighlightRole;}

  [[nodiscard]] auto disabledRole() -> std::string_view override {return popupDisabledRole;}

  private:
  Widget &owner_;
};

} // namespace jwezel::ui
