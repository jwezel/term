#pragma once

#include "list_event.hh"
#include "popup_choice.hh"
#include "widget.hh"

#include <vector>

namespace jwezel::ui {

/// A focusable choice control with a transient list popup.
///
/// Space, Enter, Down, or a click opens the choices. Arrow keys, Home/End, Page Up/Down, and the mouse wheel move the
/// highlight; Enter, Space, or a click selects the highlighted item. Escape closes the popup without changing the
/// selection. The popup closes when focus moves elsewhere.
///
/// Glyphs and colors come from the root's UI and color styles. A non-empty `caption()` changes only the closed-state
/// label; it does not change the selected item or the choices.
struct DropDown: PopupChoice {
  ///
  /// Create DropDown with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      items       The items
  /// @param      selected    Index of the selected item
  /// @param      background  The background
  DropDown(
    Container &parent,
    const Rectangle &area,
    const std::vector<Text> &items = {},
    ListIndex selected = 0,
    const Char &background = InheritBackground
  );

  ///
  /// Create DropDown with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      items       The items
  /// @param      selected    Index of the selected item
  /// @param      background  The background
  explicit DropDown(
    Container &parent,
    const std::vector<Text> &items = {},
    ListIndex selected = 0,
    const Char &background = InheritBackground
  );

  DropDown(const DropDown &) = delete;

  DropDown(DropDown &&) = delete;

  auto operator=(const DropDown &) -> DropDown & = delete;

  auto operator=(DropDown &&) -> DropDown & = delete;

  ~DropDown() override = default;

  [[nodiscard]] auto items() const -> const std::vector<Text> & {return items_;}

  ///
  /// Get an item, which must be in `[0, items().size())`
  [[nodiscard]] auto operator[](ListIndex index) const -> const Text & {
    return items_[static_cast<std::size_t>(index)];
  }

  ///
  /// Replace the items; the list is closed and the selection is limited to the new items
  void items(const std::vector<Text> &items);

  ///
  /// Get the index of the selected item
  ///
  /// @return     The index, or -1 if there are no items
  [[nodiscard]] auto selected() const -> ListIndex {return selected_;}

  ///
  /// Select an item without invoking the select handlers
  ///
  /// The index is limited to the existing items.
  void select(ListIndex index);

  ///
  /// Register a select handler
  ///
  /// The handlers are invoked when the user chooses an item, whether it is the
  /// selected one or another, with a `SelectEvent`: `event.as<SelectEvent>()`.
  ///
  /// @param[in]  handler  The handler
  ///
  /// @return     Index of the handler
  auto onSelect(const Handler &handler) -> int;

  protected:
  [[nodiscard]] auto popupItems() -> ListSource<ListIndex> override;

  [[nodiscard]] auto popupSelected() const -> ListIndex override {return selected_;}

  void popupActivate(ListIndex index, const Event &event) override;

  [[nodiscard]] auto closedText() const -> const Text * override;

  private:
  std::vector<Text> items_;
  ListIndex selected_;
};

} // namespace jwezel::ui
