#pragma once

#include "action.hh"
#include "popup_choice.hh"

#include <cstddef>
#include <vector>

namespace jwezel::ui {

///
/// An entry of a menu
///
/// The action is a weak reference: whoever creates the action keeps it alive for as long as the menu needs it. A
/// null or disabled action makes the item disabled, which navigation skips and the menu draws as such.
struct MenuItem {
  Text text;          ///< What the item shows.
  Action *action{};   ///< What choosing the item does; a weak reference.
};

///
/// A menu of actions
///
/// A menu looks like a drop-down, but choosing an item triggers its action instead of reporting a selection. The
/// popup is closed before the action runs, so an action may open a dialog, close a window, or even destroy the menu.
///
/// Navigation skips disabled items. Escape closes the menu without triggering anything.
struct Menu: PopupChoice {
  ///
  /// Create Menu with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      items       The items
  /// @param      background  The background
  Menu(
    Container &parent,
    const Rectangle &area,
    const std::vector<MenuItem> &items = {},
    const Char &background = InheritBackground
  );

  ///
  /// Create Menu with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      items       The items
  /// @param      background  The background
  Menu(
    Container &parent,
    const std::vector<MenuItem> &items = {},
    const Char &background = InheritBackground
  );

  Menu(const Menu &) = delete;

  Menu(Menu &&) = delete;

  auto operator=(const Menu &) -> Menu & = delete;

  auto operator=(Menu &&) -> Menu & = delete;

  ~Menu() override = default;

  [[nodiscard]] auto items() const -> const std::vector<MenuItem> & {return items_;}

  ///
  /// Get an item, which must be in `[0, items().size())`
  [[nodiscard]] auto operator[](ListIndex index) const -> const MenuItem & {
    return items_[static_cast<std::size_t>(index)];
  }

  ///
  /// Replace the items; the popup is closed
  void items(const std::vector<MenuItem> &items);

  protected:
  [[nodiscard]] auto popupItems() -> ListSource<ListIndex> override;

  [[nodiscard]] auto popupItemEnabled(ListIndex index) const -> bool override;

  void popupActivate(ListIndex index, const Event &event) override;

  [[nodiscard]] auto closedText() const -> const Text * override {return &caption_;}

  private:
  std::vector<MenuItem> items_;
};

} // namespace jwezel::ui
