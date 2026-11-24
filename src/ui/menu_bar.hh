#pragma once

#include "focus_group.hh"
#include "menu.hh"

#include <term/keyboard.hh>

#include <cstddef>
#include <deque>
#include <vector>

namespace jwezel::ui {

struct Root;

///
/// A row of menus that takes one stop in the focus order
///
/// The bar arranges menus next to each other and makes them one Tab stop: Tab enters at the first menu and leaves the
/// bar as a whole. Left and Right move between the menus, also while a popup is open, so that the open menu follows
/// the choice. An accelerator key such as F10 puts the focus on the bar, and a mnemonic key opens the menu it
/// belongs to from anywhere in the window.
///
/// The bar neither owns the menus nor draws anything itself: the menus are widgets in the window like any other, and
/// the application keeps them alive.
struct MenuBar {
  MenuBar() = default;

  ///
  /// Create a bar with menus
  ///
  /// @param      menus  The menus, in order; each is moved into the bar's focus group
  explicit MenuBar(std::vector<Menu *> menus);

  MenuBar(const MenuBar &) = delete;

  MenuBar(MenuBar &&) = delete;

  auto operator=(const MenuBar &) -> MenuBar & = delete;

  auto operator=(MenuBar &&) -> MenuBar & = delete;

  ~MenuBar();

  ///
  /// Add a menu at the end; nothing happens if it is in the bar already
  ///
  /// @param      menu  The menu
  void add(Menu &menu);

  ///
  /// Place the menus in a row, one row high, starting at the left edge of an area
  ///
  /// @param[in]  area     The area the menus are placed in
  /// @param[in]  spacing  Empty columns between two menus
  void arrange(const Rectangle &area, Dim spacing = 1);

  ///
  /// Move the focus to the menu next to one and optionally open it
  ///
  /// @param      from       The menu that has the focus
  /// @param[in]  direction  One for the next menu, -1 for the previous one
  /// @param[in]  open       Whether to open the menu that gets the focus
  ///
  /// @return     True if there was a menu in that direction
  auto move(Menu &from, int direction, bool open) -> bool;

  ///
  /// Give a menu the focus and optionally open it
  ///
  /// @param      menu  The menu
  /// @param[in]  open  Whether to open the menu
  void focusMenu(Menu &menu, bool open = false);

  ///
  /// Open a menu when its key is pressed in the window
  ///
  /// @param      menu  The menu
  /// @param[in]  key   The key, like `Key::AltF`
  void mnemonic(Menu &menu, Unicode key);

  ///
  /// Make a key put the focus on the bar from anywhere in the window
  ///
  /// @param[in]  key  The key, F10 by default
  void accelerate(Unicode key = Key::F10);

  ///
  /// Get the focus group that makes the bar one Tab stop
  [[nodiscard]] auto group() -> FocusGroup & {return group_;}

  ///
  /// Get the menus, in order
  [[nodiscard]] auto menus() const -> const std::vector<Menu *> & {return menus_;}

  private:
  void forget(Menu &menu);

  [[nodiscard]] auto root() -> Root *;

  std::vector<Menu *> menus_;      // weak references, in order
  std::deque<Action> actions_;     // the actions the bar owns, like the accelerator
  FocusGroup group_;

  friend struct Menu;
};

} // namespace jwezel::ui
