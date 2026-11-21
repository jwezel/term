#pragma once

#include "list_popup.hh"
#include "widget.hh"

#include <cstddef>
#include <optional>
#include <string_view>

namespace jwezel::ui {

///
/// A focusable control that opens a transient list popup
///
/// This is the part that a drop-down and a menu have in common: the closed control, the popup that opens below it,
/// and the keys and mouse gestures that move through the items. What the items are and what choosing one does is
/// left to the derived class through the protected hooks.
///
/// Space, Enter, Down, or a click opens the popup. Arrow keys, Home/End, Page Up/Down, and the mouse wheel move the
/// highlight; Enter, Space, or a click activates the highlighted item. Escape closes the popup without choosing
/// anything. The popup closes when focus moves elsewhere.
///
/// Glyphs and colors come from the root's UI and color styles. A non-empty `caption()` changes only the closed-state
/// label; it does not change the items or the selection.
struct PopupChoice: Widget {
  ///
  /// Create PopupChoice with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      background  The background
  PopupChoice(Container &parent, const Rectangle &area, const Char &background = InheritBackground);

  ///
  /// Create PopupChoice with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      background  The background
  explicit PopupChoice(Container &parent, const Char &background = InheritBackground);

  PopupChoice(const PopupChoice &) = delete;

  PopupChoice(PopupChoice &&) = delete;

  auto operator=(const PopupChoice &) -> PopupChoice & = delete;

  auto operator=(PopupChoice &&) -> PopupChoice & = delete;

  ~PopupChoice() override = default;

  ///
  /// Get the caption, which is shown instead of what was chosen while the popup is closed
  [[nodiscard]] auto caption() const -> const Text & {return caption_;}

  ///
  /// Show a fixed caption while the popup is closed
  ///
  /// This is what a menu does: it is called something, and what was chosen from it is not what it shows. The
  /// caption does not change the items or the selection, and an empty one shows what the derived class shows
  /// instead.
  ///
  /// @param[in]  caption  The caption
  void caption(const Text &caption);

  ///
  /// Open the popup; nothing happens if there are no items
  void open();

  ///
  /// Close the popup without choosing anything
  void close();

  [[nodiscard]] auto isOpen() const -> bool {return popup_.has_value();}

  ///
  /// Get the maximum number of items that are visible in the open popup
  [[nodiscard]] auto maxRows() const -> Dim {return maxRows_;}

  void maxRows(Dim rows);

  ///
  /// The popup does not follow a moved control, it is closed
  void move(const Rectangle &area) override;

  void focusChanged(bool focused) override;

  void pressedElsewhere() override;

  ///
  /// The width of the widest item or of the caption, with the glyphs around it; one row
  [[nodiscard]] auto preferredSize() -> Vector override;

  protected:
  ///
  /// The items of the popup
  [[nodiscard]] virtual auto popupItems() -> ListSource<ListIndex> = 0;

  ///
  /// The item to highlight when the popup opens
  [[nodiscard]] virtual auto popupSelected() const -> ListIndex {return 0;}

  ///
  /// Whether an item can be chosen and is not skipped by navigation
  [[nodiscard]] virtual auto popupItemEnabled(ListIndex /*index*/) const -> bool {return true;}

  ///
  /// Do what choosing an item does; the popup is closed already
  virtual void popupActivate(ListIndex index, const Event &event) = 0;

  ///
  /// The text of the closed control, or nullptr for none
  [[nodiscard]] virtual auto closedText() const -> const Text * = 0;

  void render() override;

  Text caption_;

  private:
  auto handleKey(const Event &event) -> bool;

  auto handleMouseButton(const Event &event) -> bool;

  auto handleMouseMove(const Event &event) -> bool;

  [[nodiscard]] auto firstEnabled() const -> ListIndex;

  [[nodiscard]] auto lastEnabled() const -> ListIndex;

  [[nodiscard]] auto step(ListIndex from, i8 direction) const -> ListIndex;

  [[nodiscard]] auto near(ListIndex target, i8 direction) const -> ListIndex;

  Dim maxRows_{8};
  std::size_t scanLimit_{256};
  bool dragging_{false}; // the button that opened the popup is still down
  std::optional<ListPopup<>> popup_;
};

} // namespace jwezel::ui
