#pragma once

#include <ui/button.hh>
#include <ui/horizontal_box.hh>
#include <ui/line_edit.hh>
#include <ui/list_box.hh>
#include <ui/vertical_box.hh>

#include <vector>

namespace jwezel::ui {

/// The choices or selected values of a list editor changed.
struct ListEditorEvent: UiEvent {
  enum class Change: u1 {
    choices,
    selection
  };

  ListEditorEvent(const Cause &cause, Change change, std::vector<Text> choices, std::vector<Text> selected):
  UiEvent{cause},
  change_{change},
  choices_{std::move(choices)},
  selected_{std::move(selected)}
  {}

  /// What changed: the available choices, or which choices are selected.
  [[nodiscard]] auto change() const -> Change {return change_;}

  /// The available choices after the change.
  [[nodiscard]] auto choices() const -> const std::vector<Text> & {return choices_;}

  /// The selected values after the change.
  [[nodiscard]] auto selected() const -> const std::vector<Text> & {return selected_;}

  private:
  Change change_;
  std::vector<Text> choices_;
  std::vector<Text> selected_;

  CLASS_ID(ListEditorEvent);
};

/// A multi-selection list with controls to add, edit, or remove choices.
///
/// The list and its controls are ordinary value-owned widgets. Selection is
/// independent of the available choices, and `selected()` returns copies of
/// the selected values for storage in an application model. The editor emits
/// `ListEditorEvent` for user selection and choice changes; programmatic calls
/// to `add`, `editSelected`, and `removeSelected` are silent.
struct ListEditor: HorizontalBox {
  using Handler = Element::Handler;
  using HorizontalBox::add;

  /// Create an editor with available choices and optional initial selections.
  ///
  /// @param parent Parent container; it must outlive this editor.
  /// @param choices Available values.
  /// @param selected Values selected initially; unknown values are ignored.
  ListEditor(Container &parent, std::vector<Text> choices = {}, std::vector<Text> selected = {});

  ListEditor(const ListEditor &) = delete;
  ListEditor(ListEditor &&) = delete;
  auto operator=(const ListEditor &) -> ListEditor & = delete;
  auto operator=(ListEditor &&) -> ListEditor & = delete;
  ~ListEditor() override = default;

  /// Get the available choices.
  [[nodiscard]] auto choices() const -> const std::vector<Text> & {return choices_;}

  /// Get copies of the selected values, in list order.
  [[nodiscard]] auto selected() const -> std::vector<Text>;

  /// Get the underlying list for navigation and selection operations.
  [[nodiscard]] auto list() -> ListBox<ListIndex> & {return list_;}

  /// Add a non-empty, non-duplicate choice and make it current.
  /// @return Whether a new choice was added.
  auto add(Text choice) -> bool;

  /// Replace the one selected choice with a non-empty, non-duplicate value.
  /// @return Whether the choice was changed.
  auto editSelected(Text choice) -> bool;

  /// Remove all selected choices.
  /// @return Whether any choice was removed.
  auto removeSelected() -> bool;

  /// Register a handler for a selection or choice change.
  auto onChange(const Handler &handler) -> int {return this->template on<ListEditorEvent>(handler);}

  private:
  void emitChange(const UiEvent::Cause &cause, ListEditorEvent::Change change);
  void addFromInput(const Event &event);
  void editFromInput(const Event &event);
  void removeFromInput(const Event &event);

  std::vector<Text> choices_;
  ListBox<ListIndex> list_;
  VerticalBox controls_;
  LineEdit input_;
  HorizontalBox buttons_;
  Button addButton_;
  Button editButton_;
  Button removeButton_;
};

} // namespace jwezel::ui
