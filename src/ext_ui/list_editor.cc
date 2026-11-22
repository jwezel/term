#include "list_editor.hh"

#include <algorithm>

namespace jwezel::ui {

ListEditor::ListEditor(Container &parent, std::vector<Text> choices, std::vector<Text> selected):
HorizontalBox{
  parent,
  RectangleDefault,
  BoxLayout{
    .weights = {1., 0.},
    .sizes = {LayoutSize::stretch, LayoutSize::fit},
    .alignments = {},
    .spacing = 1
  }
},
choices_{std::move(choices)},
list_{*this, listSource<ListIndex>(choices_), SelectionMode::multiple},
controls_{
  *this,
  RectangleDefault,
  BoxLayout{
    .weights = {},
    .sizes = {LayoutSize::fit, LayoutSize::fit},
    .alignments = {},
    .spacing = 0
  }
},
input_{controls_},
buttons_{
  controls_,
  RectangleDefault,
  BoxLayout{
    .weights = {},
    .sizes = {LayoutSize::fit, LayoutSize::fit, LayoutSize::fit},
    .alignments = {},
    .spacing = 1
  }
},
addButton_{buttons_, Text{"Add"}},
editButton_{buttons_, Text{"Edit"}},
removeButton_{buttons_, Text{"Remove"}}
{
  list_.maxRows(2);
  for (const auto &value: selected) {
    const auto found{std::find(choices_.begin(), choices_.end(), value)};
    if (found != choices_.end()) {
      const auto index{static_cast<ListIndex>(std::distance(choices_.begin(), found))};
      list_.addSelection(index);
      if (list_.current() < 0) {
        list_.current(index);
      }
    }
  }

  list_.onSelection(
    [this](const Element &, const Event &event) {
      emitChange(event.as<SelectionEvent>().cause(), ListEditorEvent::Change::selection);
      return true;
    }
  );
  addButton_.onPress([this](const Element &, const Event &event) {addFromInput(event); return true;});
  editButton_.onPress([this](const Element &, const Event &event) {editFromInput(event); return true;});
  removeButton_.onPress([this](const Element &, const Event &event) {removeFromInput(event); return true;});
}

auto ListEditor::selected() const -> std::vector<Text> {
  auto result{std::vector<Text>{}};
  for (auto index{ListIndex{0}}; index < list_.size(); ++index) {
    if (list_.selected(index)) {
      result.push_back(choices_[static_cast<std::size_t>(index)]);
    }
  }
  return result;
}

auto ListEditor::add(Text choice) -> bool {
  if (choice.width() == 0 or std::find(choices_.begin(), choices_.end(), choice) != choices_.end()) {
    return false;
  }
  choices_.push_back(std::move(choice));
  list_.refresh();
  const auto index{static_cast<ListIndex>(choices_.size() - 1)};
  list_.current(index);
  return true;
}

auto ListEditor::editSelected(Text choice) -> bool {
  if (choice.width() == 0) {
    return false;
  }
  auto selectedIndex{ListIndex{-1}};
  for (auto index{ListIndex{0}}; index < list_.size(); ++index) {
    if (list_.selected(index)) {
      if (selectedIndex >= 0) {
        return false;
      }
      selectedIndex = index;
    }
  }
  if (selectedIndex < 0) {
    return false;
  }
  const auto oldIndex{static_cast<std::size_t>(selectedIndex)};
  if (choice != choices_[oldIndex] and std::find(choices_.begin(), choices_.end(), choice) != choices_.end()) {
    return false;
  }
  if (choice == choices_[oldIndex]) {
    return false;
  }
  choices_[oldIndex] = std::move(choice);
  list_.refresh();
  return true;
}

auto ListEditor::removeSelected() -> bool {
  auto indices{std::vector<std::size_t>{}};
  for (auto index{ListIndex{0}}; index < list_.size(); ++index) {
    if (list_.selected(index)) {
      indices.push_back(static_cast<std::size_t>(index));
    }
  }
  if (indices.empty()) {
    return false;
  }
  for (auto index{indices.rbegin()}; index != indices.rend(); ++index) {
    choices_.erase(choices_.begin() + static_cast<std::ptrdiff_t>(*index));
  }
  list_.refresh();
  list_.clearSelection();
  return true;
}

void ListEditor::emitChange(const UiEvent::Cause &cause, ListEditorEvent::Change change) {
  emit(ListEditorEvent{cause, change, choices_, selected()});
}

void ListEditor::addFromInput(const Event &event) {
  if (add(Text{input_.contents()})) {
    input_.contents("");
    emitChange(event.as<PressEvent>().cause(), ListEditorEvent::Change::choices);
  }
}

void ListEditor::editFromInput(const Event &event) {
  if (editSelected(Text{input_.contents()})) {
    input_.contents("");
    emitChange(event.as<PressEvent>().cause(), ListEditorEvent::Change::choices);
  }
}

void ListEditor::removeFromInput(const Event &event) {
  if (removeSelected()) {
    emitChange(event.as<PressEvent>().cause(), ListEditorEvent::Change::choices);
  }
}

} // namespace jwezel::ui
