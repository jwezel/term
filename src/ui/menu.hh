#pragma once

#include "action.hh"
#include "popup_choice.hh"

#include <cstddef>
#include <deque>
#include <vector>

namespace jwezel::ui {

struct Menu;
struct MenuBar;

///
/// An entry of a menu
///
/// The action is a weak reference: whoever creates the action keeps it alive for as long as the menu needs it. A
/// null or disabled action, and a separator, make the item unavailable: navigation skips those and the menu draws
/// them as such.
///
/// An item can have a hint, which is drawn right-aligned, and can be checked, which puts a mark in front of it. An
/// item that refers to a submenu opens that menu next to it instead of triggering an action.
struct MenuItem {
  Text text;          ///< What the item shows.
  Action *action{};   ///< What choosing the item does; a weak reference.
  Text hint{};        ///< A short cut or other hint, drawn right-aligned.
  Menu *submenu{};    ///< A submenu to open; a weak reference.
  bool checked{};     ///< Whether the item is marked as chosen.
  bool separator{};   ///< Whether the item is a rule between items.

  ///
  /// Create a separator
  [[nodiscard]] static auto rule() -> MenuItem;

  ///
  /// Create an item that opens a submenu
  ///
  /// @param[in]  text     What the item shows
  /// @param      submenu  The submenu, which must outlive the menu that shows the item
  [[nodiscard]] static auto withSubmenu(const Text &text, Menu &submenu) -> MenuItem;
};

///
/// A menu of actions, separators, and submenus
///
/// A menu looks like a drop-down, but choosing an item triggers its action instead of reporting a selection. The
/// popup is closed before the action runs, so an action may open a dialog, close a window, or even destroy the menu.
///
/// Navigation skips disabled items and separators. Escape closes the menu without triggering anything. Right opens a
/// submenu, Left or Escape closes it again and returns to the item that opened it.
///
/// A menu that belongs to a menu bar moves to its neighbour there with Left and Right when no submenu is open. A
/// mnemonic key opens the menu from anywhere in its window.
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

  ~Menu() override;

  [[nodiscard]] auto items() const -> const std::vector<MenuItem> & {return items_;}

  ///
  /// Get an item, which must be in `[0, items().size())`
  [[nodiscard]] auto operator[](ListIndex index) const -> const MenuItem & {
    return items_[static_cast<std::size_t>(index)];
  }

  ///
  /// Replace the items; the popup and its submenus are closed
  void items(const std::vector<MenuItem> &items);

  ///
  /// Whether an item is checked, which must be in `[0, items().size())`
  [[nodiscard]] auto checked(ListIndex index) const -> bool;

  ///
  /// Set the check mark of an item and show it
  ///
  /// @param[in]  index    The item, which must be in `[0, items().size())`
  /// @param[in]  checked  Whether the item is marked as chosen
  void checked(ListIndex index, bool checked);

  ///
  /// Open the menu when the key is pressed in the window
  ///
  /// The menu owns the action that does it; nothing happens if it is not in a window.
  ///
  /// @param[in]  key  The key, like `Key::AltF`
  void mnemonic(Unicode key);

  ///
  /// Get the menu bar this menu belongs to, if any
  [[nodiscard]] auto bar() const -> MenuBar * {return bar_;}

  ///
  /// The width of the closed control: what the menu is called, with the glyphs around it; one row
  [[nodiscard]] auto preferredSize() -> Vector override;

  protected:
  [[nodiscard]] auto popupItems() -> ListSource<ListIndex> override;

  [[nodiscard]] auto popupItemEnabled(ListIndex index) const -> bool override;

  void popupActivate(ListIndex index, const Event &event) override;

  [[nodiscard]] auto closedText() const -> const Text * override;

  [[nodiscard]] auto closedGlyphs(std::size_t state) -> const Brackets & override {
    return uiStyle().menu[state];
  }

  [[nodiscard]] auto closedRole(std::size_t state) -> std::string_view override {
    return menuRoles[state];
  }

  auto handleKey(const Event &event) -> bool override;

  void close() override;

  private:
  void rebuild();

  void hostShortcuts();

  void clearShortcuts();

  void openChild(Menu &submenu, ListIndex index);

  void closeChild();

  std::vector<MenuItem> items_;
  std::vector<Text> rows_;           // the popup rows, with marks, hints, and rules
  MenuBar *bar_{};                   // weak reference
  Menu *child_{};                    // weak reference to the open submenu
  Menu *parentMenu_{};               // weak reference to the menu that opened this one
  Action mnemonicAction_;            // the action that opens the menu from its shortcut
  std::deque<Action> shortcutActions_; // the accelerators of the items; each closes the menus and triggers its action

  friend struct MenuBar;
};

} // namespace jwezel::ui
