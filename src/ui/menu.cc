#include "menu.hh"

namespace jwezel::ui {

Menu::Menu(Container &parent, const Rectangle &area, const std::vector<MenuItem> &items, const Char &background):
PopupChoice{parent, area, background},
items_{items}
{
  render();
  layoutChanged();
}

Menu::Menu(Container &parent, const std::vector<MenuItem> &items, const Char &background):
PopupChoice{parent, background},
items_{items}
{
  render();
  layoutChanged();
}

void Menu::items(const std::vector<MenuItem> &items) {
  close();
  items_ = items;
  render();
  layoutChanged();
}

auto Menu::popupItems() -> ListSource<ListIndex> {
  return ListSource<ListIndex>{
    [this] {return static_cast<ListIndex>(items_.size());},
    [this](ListIndex index) {return items_[static_cast<std::size_t>(index)].text;}
  };
}

auto Menu::popupItemEnabled(ListIndex index) const -> bool {
  const auto *action{items_[static_cast<std::size_t>(index)].action};
  return action and action->enabled();
}

void Menu::popupActivate(ListIndex index, const Event & /*event*/) {
  auto *action{items_[static_cast<std::size_t>(index)].action};
  close();
  if (action) {
    action->trigger();
  }
}

} // namespace jwezel::ui
