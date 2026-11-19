#pragma once

#include "list_popup.hh"
#include "ui_event.hh"
#include "widget.hh"

#include <functional>
#include <optional>
#include <vector>

namespace jwezel::ui {

///
/// Drop down control
///
/// Shows the selected item, framed like a button and with an arrow. Space,
/// Enter, the down key or a click open a list with all items below it. In the
/// open list the arrow keys, Home/End, Page up/down and the mouse wheel move
/// the highlight, Enter or Space choose the highlighted item, and Escape
/// closes the list without choosing. With the mouse an item is chosen by
/// clicking it, or by pressing the drop down, moving to the item and releasing
/// the button. The list closes when the focus goes elsewhere or when the mouse
/// is clicked somewhere else.
///
/// The glyphs around the selected item come from the UI style (by state: normal,
/// focus, and active while the list is open), and so do the colors of the roles
/// `dropdown`, `dropdown.focus` and `dropdown.active`, which are tinted by default
/// when the drop down has the focus.
///
/// The list is a popup in front of all other elements, which only exists while it
/// is open. Only the first row of the items is shown.
///
/// An item of a drop down was chosen
///
/// The drop down has selected the item when the event is emitted; `previous()` is what it had selected, so
/// the choice changed something if `changed()`.
struct SelectEvent: UiEvent {
  SelectEvent(const Cause &cause, Dim index, Dim previous, Text item):
  UiEvent{cause},
  index_{index},
  previous_{previous},
  item_{std::move(item)}
  {}

  ///
  /// The index of the chosen item
  [[nodiscard]] auto index() const -> Dim {return index_;}

  ///
  /// The index of the item that was selected before, which may be the same
  [[nodiscard]] auto previous() const -> Dim {return previous_;}

  ///
  /// The chosen item
  [[nodiscard]] auto item() const -> const Text & {return item_;}

  ///
  /// Whether another item than the selected one was chosen
  [[nodiscard]] auto changed() const -> bool {return index_ != previous_;}

  private:
  Dim index_;
  Dim previous_;
  Text item_;

  CLASS_ID(SelectEvent);
};

struct DropDown: Widget {
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
    Dim selected = 0,
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
    Dim selected = 0,
    const Char &background = InheritBackground
  );

  DropDown(const DropDown &) = delete;

  DropDown(DropDown &&) = delete;

  auto operator=(const DropDown &) -> DropDown & = delete;

  auto operator=(DropDown &&) -> DropDown & = delete;

  ~DropDown() override = default;

  [[nodiscard]] auto items() const -> const std::vector<Text> & {return items_;}

  ///
  /// Replace the items; the list is closed and the selection is limited to the new items
  void items(const std::vector<Text> &items);

  ///
  /// Get the caption, which is shown instead of the selected item while the list is closed
  [[nodiscard]] auto caption() const -> const Text & {return caption_;}

  ///
  /// Show a fixed caption instead of the selected item while the list is closed
  ///
  /// This is what a menu does: it is called something, and what was chosen from it is not what it shows. The
  /// caption does not change the items or the selection, and an empty one shows the selected item again.
  ///
  /// @param[in]  caption  The caption
  void caption(const Text &caption);

  ///
  /// Get the index of the selected item
  ///
  /// @return     The index, or -1 if there are no items
  [[nodiscard]] auto selected() const -> Dim {return selected_;}

  ///
  /// Select an item without invoking the select handlers
  ///
  /// The index is limited to the existing items.
  void select(Dim index);

  ///
  /// Open the list; nothing happens if there are no items
  void open();

  ///
  /// Close the list without choosing anything
  void close();

  [[nodiscard]] auto isOpen() const -> bool {return popup_.has_value();}

  ///
  /// Get the maximum number of items that are visible in the open list
  [[nodiscard]] auto maxRows() const -> Dim {return maxRows_;}

  void maxRows(Dim rows);

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

  ///
  /// The open list does not follow a moved drop down, it is closed
  void move(const Rectangle &area) override;

  void focusChanged(bool focused) override;

  void pressedElsewhere() override;

  ///
  /// The width of the widest item or of the caption, with the glyphs around it; one row
  [[nodiscard]] auto preferredSize() -> Vector override;

  protected:
  void render() override;

  private:
  void registerHandlers();

  auto handleKey(const Event &event) -> bool;

  auto handleMouseButton(const Event &event) -> bool;

  auto handleMouseMove(const Event &event) -> bool;

  void choose(Dim index, const Event &event);

  std::vector<Text> items_;
  Text caption_;
  Dim selected_;
  Dim maxRows_{8};
  bool dragging_{false}; // the button that opened the list is still down
  std::optional<ListPopup> popup_;
};

} // namespace jwezel::ui
