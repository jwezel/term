#pragma once

#include "list_event.hh"
#include "list_view.hh"

#include <algorithm>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace jwezel::ui {

///
/// How a list box handles the selection
enum class SelectionMode: u1 {
  single,  ///< One item is selected, and moving the current item moves the selection.
  multiple ///< Any number of items can be selected.
};

///
/// A focusable, scrolling list of items
///
/// The items come from a `ListSource`, which can be lazy: a database-backed list only has to tell its size and
/// produce the rows that are visible. A small list can choose a narrow index type, so `ListBox<i1>` keeps its
/// current row, scroll position, and selected indices in a byte each.
///
/// In `SelectionMode::multiple`, arrows move the current item, Space selects only that item, and Shift with a movement
/// selects the inclusive range from the anchor row through the current row. Ctrl+A selects all enabled items; Escape
/// clears a non-empty selection, and otherwise remains available to a window's cancel action. A click selects only the
/// clicked item, Ctrl+click toggles it, and Shift+click selects an inclusive range. An unmodified drag replaces the
/// selection with the rows crossed; Ctrl-drag extends the anchored selection. Enter activates the current item. The
/// mouse wheel scrolls the view without moving the current item.
///
/// In `SelectionMode::single`, moving the current item selects it and emits a `SelectEvent`, like a drop-down does.
/// Multiple mode emits a `SelectionEvent` for every item whose selected state changes, and `ActivateEvent` when an
/// item is activated. Selection is not activation: Enter does not change the selection.
///
/// @tparam     Index  The index type, which is the one the source reports from `size()`
template<std::signed_integral Index = ListIndex>
struct ListBox: ListView<Index> {
  using Handler = Element::Handler;
  ///
  /// Create ListBox with an explicit area
  ///
  /// @param      parent        The parent container
  /// @param      area          The area; as many rows as it has are visible at a time
  /// @param      source        The items
  /// @param      mode          Whether one or many items can be selected
  /// @param      currentIndex  Index of the current item
  /// @param      background    The background
  ListBox(
    Container &parent,
    const Rectangle &area,
    ListSource<Index> source,
    SelectionMode mode = SelectionMode::multiple,
    Index currentIndex = Index{0},
    const Char &background = InheritBackground
  ):
  ListView<Index>{parent, area, std::move(source), currentIndex, background},
  mode_{mode}
  {
    initialize();
  }

  ///
  /// Create ListBox with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent        The parent container
  /// @param      source        The items
  /// @param      mode          Whether one or many items can be selected
  /// @param      currentIndex  Index of the current item
  /// @param      background    The background
  ListBox(
    Container &parent,
    ListSource<Index> source,
    SelectionMode mode = SelectionMode::multiple,
    Index currentIndex = Index{0},
    const Char &background = InheritBackground
  ):
  ListView<Index>{parent, RectangleDefault, std::move(source), currentIndex, background},
  mode_{mode}
  {
    initialize();
  }

  ListBox(const ListBox &) = delete;

  ListBox(ListBox &&) = delete;

  auto operator=(const ListBox &) -> ListBox & = delete;

  auto operator=(ListBox &&) -> ListBox & = delete;

  ~ListBox() override = default;

  ///
  /// Get whether one or many items can be selected
  [[nodiscard]] auto selectionMode() const -> SelectionMode {return mode_;}

  ///
  /// Get this list's item-selection style, or the UI style's default when not overridden
  [[nodiscard]] auto itemSelectionStyle() -> ItemSelectionStyle {
    return itemSelectionStyle_.value_or(this->uiStyle().itemSelectionStyle);
  }

  ///
  /// Set this list's item-selection style, overriding the UI style's default
  void itemSelectionStyle(ItemSelectionStyle style) {
    itemSelectionStyle_ = style;
    this->render();
    this->layoutChanged();
  }

  ///
  /// Use the item-selection style from the UI style again
  void useUiItemSelectionStyle() {
    itemSelectionStyle_.reset();
    this->render();
    this->layoutChanged();
  }

  ///
  /// Set whether one or many items can be selected
  ///
  /// Switching to single mode keeps only the current item selected. This is a programmatic change and is silent.
  void selectionMode(SelectionMode mode) {
    if (mode == mode_) {
      return;
    }
    mode_ = mode;
    resetRange();
    this->anchor(this->current());
    if (mode_ == SelectionMode::single) {
      this->ListView<Index>::select(this->current());
    }
    this->render();
  }

  ///
  /// Select only one item and make it the current one
  ///
  /// The selection is changed without telling anyone; this is the programmatic counterpart of what the user does.
  void select(Index index) {
    resetRange();
    this->ListView<Index>::select(index);
    if (index >= 0 and index < this->size()) {
      this->current(index);
      this->anchor(index);
    }
  }

  ///
  /// Select every enabled item without emitting selection events
  void selectAll() {
    resetRange();
    this->ListView<Index>::clearSelection();
    for (auto index{Index{0}}; index < this->size(); ++index) {
      if (this->itemEnabled(index)) {
        this->ListView<Index>::addSelection(index);
      }
    }
  }

  ///
  /// Deselect one item without emitting a selection event
  auto deselect(Index index) -> bool {
    resetRange();
    return this->ListView<Index>::removeSelection(index);
  }

  ///
  /// Deselect every item without emitting selection events
  void clearSelection() {
    resetRange();
    this->ListView<Index>::clearSelection();
  }

  ///
  /// Add an item to the selection without emitting an event
  auto addSelection(Index index) -> bool {
    resetRange();
    return this->ListView<Index>::addSelection(index);
  }

  ///
  /// Remove an item from the selection without emitting an event
  auto removeSelection(Index index) -> bool {
    resetRange();
    return this->ListView<Index>::removeSelection(index);
  }

  ///
  /// Toggle an item's selected state without emitting an event
  auto toggle(Index index) -> bool {
    resetRange();
    return this->ListView<Index>::toggle(index);
  }

  ///
  /// Change a range of selected items without emitting events
  void selectRange(Index from, Index to, bool selected = true) {
    resetRange();
    this->ListView<Index>::selectRange(from, to, selected);
  }

  ///
  /// Register a handler for the single-choice selection of single mode
  auto onSelect(const Handler &handler) -> int {return this->template on<SelectEvent>(handler);}

  ///
  /// Register a handler for an item that was added to or removed from the selection
  auto onSelection(const Handler &handler) -> int {return this->template on<SelectionEvent>(handler);}

  ///
  /// Register a handler for an activated item
  auto onActivate(const Handler &handler) -> int {return this->template on<ActivateEvent>(handler);}

  protected:
  [[nodiscard]] auto itemGlyphs() -> const Brackets & override {return this->uiStyle().list;}

  [[nodiscard]] auto backgroundRole() -> std::string_view override {
    return this->focused()? listFocusRole: listRole;
  }

  [[nodiscard]] auto currentRole() -> std::string_view override {
    if (itemSelectionStyle() == ItemSelectionStyle::caret) {
      return this->focused()? listFocusRole: listRole;
    }
    return this->focused()? listCurrentRole: listRole;
  }

  [[nodiscard]] auto currentSelectedRole() -> std::string_view override {
    return this->focused()? listCurrentSelectedRole: listSelectedRole;
  }

  [[nodiscard]] auto marker() -> std::string_view override {
    return itemSelectionStyle() == ItemSelectionStyle::caret?
      this->uiStyle().listMarker:
      std::string_view{};
  }

  [[nodiscard]] auto markerVisible() -> bool override {
    return this->focused() and itemSelectionStyle() == ItemSelectionStyle::caret;
  }

  private:
  void initialize() {
    this->anchor(this->current());
    if (mode_ == SelectionMode::single) {
      this->ListView<Index>::select(this->current());
    }
    this->render();
    this->onKey([this](const Element &, const Event &event) {return handleKey(event);});
    this->onMouseButton([this](const Element &, const Event &event) {return handleMouseButton(event);});
    this->onMouseMove([this](const Element &, const Event &event) {return handleMouseMove(event);});
  }

  [[nodiscard]] auto lastIndex() const -> Index {return static_cast<Index>(this->size() - 1);}

  [[nodiscard]] auto step(Index from, i8 direction) -> Index {
    if (this->size() <= 0) {
      return static_cast<Index>(-1);
    }
    auto index{from};
    for (;;) {
      const auto candidate{static_cast<Index>(static_cast<i8>(index) + direction)};
      if (candidate < 0 or candidate >= this->size()) {
        return from;
      }
      index = candidate;
      if (this->itemEnabled(index)) {
        return index;
      }
    }
  }

  [[nodiscard]] auto edge(bool first) -> Index {
    if (this->size() <= 0) {
      return static_cast<Index>(-1);
    }
    if (first) {
      for (auto index{Index{0}}; index < this->size(); ++index) {
        if (this->itemEnabled(index)) {
          return index;
        }
      }
    } else {
      for (auto index{lastIndex()};; --index) {
        if (this->itemEnabled(index)) {
          return index;
        }
        if (index == 0) {
          break;
        }
      }
    }
    return static_cast<Index>(-1);
  }

  [[nodiscard]] auto near(i8 target, i8 direction) -> Index {
    if (this->size() <= 0) {
      return static_cast<Index>(-1);
    }
    target = std::clamp(target, i8{0}, static_cast<i8>(lastIndex()));
    if (this->itemEnabled(static_cast<Index>(target))) {
      return static_cast<Index>(target);
    }
    auto index{target};
    for (;;) {
      const auto candidate{static_cast<i8>(index + direction)};
      if (candidate < 0 or candidate >= this->size()) {
        return static_cast<Index>(-1);
      }
      index = candidate;
      if (this->itemEnabled(static_cast<Index>(index))) {
        return static_cast<Index>(index);
      }
    }
  }

  void tellSelection(Index index, bool selected, const Event &event) {
    tellSelection(index, selected, UiEvent::causeOf(event));
  }

  void tellSelection(Index index, bool selected, const UiEvent::Cause &cause) {
    this->emit(
      SelectionEvent{
        cause,
        static_cast<ListIndex>(index),
        selected,
        this->item(index)
      }
    );
  }

  void tellSelect(Index index, Index previous, const Event &event) {
    this->emit(
      SelectEvent{
        UiEvent::causeOf(event),
        static_cast<ListIndex>(index),
        static_cast<ListIndex>(previous),
        this->item(index)
      }
    );
  }

  void extendTo(Index oldCurrent, Index newCurrent, const UiEvent::Cause &cause) {
    if (not rangeActive_) {
      rangeAnchor_ = this->anchor() >= 0? this->anchor(): oldCurrent;
      rangeBase_ = this->selection();
      rangeActive_ = true;
    }
    auto target{rangeBase_};
    const auto from{std::min(rangeAnchor_, newCurrent)}, to{std::max(rangeAnchor_, newCurrent)};
    for (auto index{from};; ++index) {
      if (this->itemEnabled(index)) {
        target.add(index);
      }
      if (index == to) {
        break;
      }
    }
    applySelection(target, cause);
  }

  void applySelection(const Selection<Index> &target, const UiEvent::Cause &cause) {
    std::vector<std::pair<Index, bool>> changes;
    for (const auto index: this->selection()) {
      if (not target.contains(index)) {
        changes.emplace_back(index, false);
      }
    }
    for (const auto index: target) {
      if (not this->selected(index)) {
        changes.emplace_back(index, true);
      }
    }
    std::sort(changes.begin(), changes.end(), [](const auto &left, const auto &right) {
      return left.first < right.first;
    });
    for (const auto &[index, selected]: changes) {
      if (selected) {
        this->ListView<Index>::addSelection(index);
      } else {
        this->ListView<Index>::removeSelection(index);
      }
      tellSelection(index, selected, cause);
    }
  }

  void selectAllByUser(const Event &event) {
    Selection<Index> target;
    for (auto index{Index{0}}; index < this->size(); ++index) {
      if (this->itemEnabled(index)) {
        target.add(index);
      }
    }
    resetRange();
    applySelection(target, UiEvent::causeOf(event));
  }

  void clearSelectionByUser(const Event &event) {
    resetRange();
    applySelection(Selection<Index>{}, UiEvent::causeOf(event));
  }

  void resetRange() {
    rangeActive_ = false;
    rangeAnchor_ = static_cast<Index>(-1);
    rangeBase_.clear();
  }

  void moveCurrent(Index index, const Event &event, bool extend) {
    if (index < 0 or index >= this->size()) {
      return;
    }
    const auto previous{this->current()};
    if (index == previous) {
      if (mode_ == SelectionMode::multiple and extend) {
        extendTo(previous, index, UiEvent::causeOf(event));
      }
      return;
    }
    this->current(index);
    if (mode_ == SelectionMode::single) {
      this->ListView<Index>::select(index);
      tellSelect(index, previous, event);
      this->anchor(index);
    } else if (extend) {
      extendTo(previous, index, UiEvent::causeOf(event));
    } else {
      resetRange();
      this->anchor(index);
    }
  }

  auto handleKey(const Event &event) -> bool {
    const auto key{event.as<KeyEvent>().key()};
    const auto current{this->current()};
    const auto page{std::max(Dim{1}, this->rows())};
    switch (key) {
      case Key::Up:
      moveCurrent(step(current, -1), event, false);
      return true;

      case Key::Down:
      moveCurrent(step(current, 1), event, false);
      return true;

      case Key::ShiftUp:
      moveCurrent(step(current, -1), event, true);
      return true;

      case Key::ShiftDown:
      moveCurrent(step(current, 1), event, true);
      return true;

      case Key::Home:
      moveCurrent(edge(true), event, false);
      return true;

      case Key::End:
      moveCurrent(edge(false), event, false);
      return true;

      case Key::ShiftHome:
      moveCurrent(edge(true), event, true);
      return true;

      case Key::ShiftEnd:
      moveCurrent(edge(false), event, true);
      return true;

      case Key::PageUp:
      moveCurrent(near(static_cast<i8>(current) - page, -1), event, false);
      return true;

      case Key::PageDown:
      moveCurrent(near(static_cast<i8>(current) + page, 1), event, false);
      return true;

      case Key::ShiftPageUp:
      moveCurrent(near(static_cast<i8>(current) - page, -1), event, true);
      return true;

      case Key::ShiftPageDown:
      moveCurrent(near(static_cast<i8>(current) + page, 1), event, true);
      return true;

      case ' ':
      if (mode_ == SelectionMode::multiple and current >= 0 and this->itemEnabled(current)) {
        resetRange();
        replaceSelection(current, event);
        this->anchor(current);
      }
      return true;

      case '\x01': // Ctrl+A
      if (mode_ == SelectionMode::multiple) {
        selectAllByUser(event);
        return true;
      }
      return false;

      case Key::Escape:
      if (mode_ == SelectionMode::multiple and not this->selection().empty()) {
        clearSelectionByUser(event);
        return true;
      }
      return false;

      case Key::Enter:
      if (current >= 0 and this->itemEnabled(current)) {
        this->emit(
          ActivateEvent{
            UiEvent::causeOf(event),
            static_cast<ListIndex>(current),
            this->item(current)
          }
        );
      }
      return true;

      default:
      return false;
    }
  }

  auto handleMouseButton(const Event &event) -> bool {
    const auto &mouse{event.as<MouseButtonEvent>()};
    if (mouse.modifiers().mod5) { // the wheel: button 1 is up, button 2 is down
      if (mouse.action() == MouseAction::Down) {
        this->scroll(mouse.button() == MouseButton::Button1? -1: 1);
        return true;
      }
      return false;
    }
    if (mouse.button() != MouseButton::Button1) {
      return false;
    }
    if (mouse.action() == MouseAction::Up) {
      if (pendingControlClick_) {
        const auto index{pendingControlIndex_};
        const auto selected{this->toggle(index)};
        tellSelection(index, selected, UiEvent::Cause{*pendingControlCause_});
        this->anchor(index);
        resetPendingControlClick();
        return true;
      }
      if (dragging_) {
        dragging_ = false;
        return true;
      }
      return false;
    }
    const auto index{this->itemAt(mouse.position())};
    if (index < 0 or not this->itemEnabled(index)) {
      return false;
    }
    const auto previous{this->current()};
    this->current(index);
    if (mode_ == SelectionMode::multiple and mouse.modifiers().control) {
      resetRange();
      pendingControlClick_ = true;
      pendingControlIndex_ = index;
      pendingControlAnchor_ = index;
      pendingControlBase_ = this->selection();
      pendingControlCause_.emplace(mouse);
    } else if (mode_ == SelectionMode::multiple and mouse.modifiers().shift) {
      resetRange();
      extendTo(previous, index, UiEvent::causeOf(event));
    } else {
      if (mode_ == SelectionMode::single) {
        this->ListView<Index>::select(index);
      } else {
        resetRange();
        rangeActive_ = true;
        rangeAnchor_ = index;
        rangeBase_.clear();
        this->anchor(index);
        dragging_ = true;
        extendTo(index, index, UiEvent::causeOf(event));
      }
      if (mode_ == SelectionMode::single and index != previous) {
        tellSelect(index, previous, event);
      }
      this->anchor(index);
    }
    return true;
  }

  auto handleMouseMove(const Event &event) -> bool {
    if (not dragging_ and not pendingControlClick_) {
      return false;
    }
    const auto index{dragItemAt(event.as<MouseMoveEvent>().position())};
    const auto previous{this->current()};
    if (pendingControlClick_) {
      rangeAnchor_ = pendingControlAnchor_ >= 0? pendingControlAnchor_: pendingControlIndex_;
      rangeBase_ = pendingControlBase_;
      rangeActive_ = true;
      dragging_ = true;
      resetPendingControlClick();
    }
    if (index < 0 or not this->itemEnabled(index)) {
      return true;
    }
    if (index != previous) {
      this->current(index);
    }
    extendTo(previous, index, UiEvent::causeOf(event));
    return true;
  }

  void resetPendingControlClick() {
    pendingControlClick_ = false;
    pendingControlIndex_ = static_cast<Index>(-1);
    pendingControlAnchor_ = static_cast<Index>(-1);
    pendingControlBase_.clear();
    pendingControlCause_.reset();
  }

  [[nodiscard]] auto dragItemAt(const Vector &position) -> Index {
    const auto bounds{this->bounds()};
    if (bounds.width() <= 0 or bounds.height() <= 0) {
      return static_cast<Index>(-1);
    }
    const auto clamped{Vector{
      std::clamp(position.x(), bounds.x1(), static_cast<Dim>(bounds.x2() - 1)),
      std::clamp(position.y(), bounds.y1(), static_cast<Dim>(bounds.y2() - 1))
    }};
    return this->itemAt(clamped);
  }

  void replaceSelection(Index index, const Event &event) {
    std::vector<std::pair<Index, bool>> changes;
    for (const auto selected: this->selection()) {
      if (selected != index) {
        changes.emplace_back(selected, false);
      }
    }
    if (not this->selected(index)) {
      changes.emplace_back(index, true);
    }
    this->ListView<Index>::select(index);
    std::sort(changes.begin(), changes.end(), [](const auto &left, const auto &right) {
      return left.first < right.first;
    });
    for (const auto &[changed, selected]: changes) {
      tellSelection(changed, selected, event);
    }
    resetRange();
  }

  SelectionMode mode_;
  std::optional<ItemSelectionStyle> itemSelectionStyle_;
  bool dragging_{false};
  bool pendingControlClick_{false};
  Index pendingControlIndex_{static_cast<Index>(-1)};
  Index pendingControlAnchor_{static_cast<Index>(-1)};
  Selection<Index> pendingControlBase_;
  std::optional<MouseButtonEvent> pendingControlCause_;
  bool rangeActive_{false};
  Index rangeAnchor_{static_cast<Index>(-1)};
  Selection<Index> rangeBase_;
};

} // namespace jwezel::ui
