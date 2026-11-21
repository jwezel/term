#include "drop_down.hh"
#include "root.hh"

#include <algorithm>

#include <term/event.hh>

namespace jwezel::ui {

namespace {

auto limit(ListIndex index, const std::vector<Text> &items) -> ListIndex {
  return items.empty()? ListIndex{-1}: std::clamp(index, ListIndex{0}, static_cast<ListIndex>(items.size() - 1));
}

} // namespace

DropDown::DropDown(Container &parent, const Rectangle &area, const std::vector<Text> &items, ListIndex selected, const Char &background):
PopupChoice{parent, area, background},
items_{items},
selected_{limit(selected, items)}
{
  render();
  layoutChanged();
}

DropDown::DropDown(Container &parent, const std::vector<Text> &items, ListIndex selected, const Char &background):
PopupChoice{parent, background},
items_{items},
selected_{limit(selected, items)}
{
  render();
  layoutChanged();
}

void DropDown::items(const std::vector<Text> &items) {
  close();
  items_ = items;
  selected_ = limit(selected_, items_);
  render();
  layoutChanged();
}

void DropDown::select(ListIndex index) {
  selected_ = limit(index, items_);
  render();
  layoutChanged();
}

auto DropDown::onSelect(const Handler &handler) -> int {
  return on<SelectEvent>(handler);
}

auto DropDown::popupItems() -> ListSource<ListIndex> {
  return listSource(items_);
}

void DropDown::popupActivate(ListIndex index, const Event &event) {
  const auto previous{selected_};
  select(index);
  close();
  emit(SelectEvent{UiEvent::causeOf(event), selected_, previous, items_[static_cast<size_t>(selected_)]});
}

auto DropDown::closedText() const -> const Text * {
  if (caption_.width() > 0) {
    return &caption_;
  }
  if (selected_ >= 0) {
    return &items_[static_cast<size_t>(selected_)];
  }
  return nullptr;
}

} // namespace jwezel::ui
