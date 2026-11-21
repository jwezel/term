#pragma once

#include "list_source.hh"
#include "ui_event.hh"

#include <string>
#include <utility>

namespace jwezel::ui {

///
/// Notification of a single-choice item choice
///
/// `SelectEvent` is emitted by widgets that have one selected item: `DropDown` when the user chooses an item, and
/// `ListBox` in single-selection mode when the current selection moves. It describes a replacement of the selection,
/// so it carries both the new `index` and the `previous` one. A drop-down reports a user choosing the already selected
/// item too; use `changed()` to see whether the indices differ.
///
/// For a multi-selection list, `SelectionEvent` reports each item that was added to or removed from the selection
/// instead; the two events are not interchangeable.
struct SelectEvent: UiEvent {
  /// Describe the selected value and the selection that preceded it.
  /// @param cause Key or mouse input that selected the item.
  /// @param index Index now selected.
  /// @param previous Index selected before this event.
  /// @param item Copy of the selected item text.
  SelectEvent(const Cause &cause, ListIndex index, ListIndex previous, Text item):
  UiEvent{cause},
  index_{index},
  previous_{previous},
  item_{std::move(item)}
  {}

  ///
  /// The index of the chosen item
  [[nodiscard]] auto index() const -> ListIndex {return index_;}

  ///
  /// The index of the item that was selected before, which may be the same
  [[nodiscard]] auto previous() const -> ListIndex {return previous_;}

  ///
  /// The chosen item
  [[nodiscard]] auto item() const -> const Text & {return item_;}

  ///
  /// Whether another item than the selected one was chosen
  [[nodiscard]] auto changed() const -> bool {return index_ != previous_;}

  private:
  ListIndex index_;
  ListIndex previous_;
  Text item_;

  CLASS_ID(SelectEvent);
};

///
/// Notification that one item was added to or removed from a multi-selection
///
/// `ListBox` in multiple-selection mode emits one `SelectionEvent` for every index whose selected state changed,
/// in ascending index order. The event is a delta: it says which item changed and in which direction, not what the
/// whole selection is. Query `ListBox::selection()` for the full set.
///
/// Programmatic changes through `select`, `toggle`, `clearSelection`, and `refresh` are silent; only user input
/// emits events.
struct SelectionEvent: UiEvent {
  /// Describe one changed item.
  /// @param cause Key or mouse input that changed the selection.
  /// @param index Index whose state changed.
  /// @param selected Whether the item is selected now.
  /// @param item Copy of the item text.
  SelectionEvent(const Cause &cause, ListIndex index, bool selected, Text item):
  UiEvent{cause},
  index_{index},
  selected_{selected},
  item_{std::move(item)}
  {}

  ///
  /// The index whose selected state changed
  [[nodiscard]] auto index() const -> ListIndex {return index_;}

  ///
  /// Whether the item is selected now
  [[nodiscard]] auto selected() const -> bool {return selected_;}

  ///
  /// The item
  [[nodiscard]] auto item() const -> const Text & {return item_;}

  private:
  ListIndex index_;
  bool selected_;
  Text item_;

  CLASS_ID(SelectionEvent);
};

///
/// Notification that the current list item was activated
///
/// Activation is not selection: `ListBox` emits it when the user presses Enter on the current item, no matter how
/// many items are selected. The item is the current one, not the whole selection.
struct ActivateEvent: UiEvent {
  /// Describe the activated item.
  /// @param cause Key or mouse input that activated the item.
  /// @param index Index of the activated item.
  /// @param item Copy of the item text.
  ActivateEvent(const Cause &cause, ListIndex index, Text item):
  UiEvent{cause},
  index_{index},
  item_{std::move(item)}
  {}

  ///
  /// The index of the activated item
  [[nodiscard]] auto index() const -> ListIndex {return index_;}

  ///
  /// The activated item
  [[nodiscard]] auto item() const -> const Text & {return item_;}

  private:
  ListIndex index_;
  Text item_;

  CLASS_ID(ActivateEvent);
};

} // namespace jwezel::ui
