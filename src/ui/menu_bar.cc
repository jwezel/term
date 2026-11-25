#include "menu_bar.hh"

#include "root.hh"

#include <algorithm>

namespace jwezel::ui {

MenuBar::MenuBar(std::vector<Menu *> menus) {
  for (auto *menu: menus) {
    if (menu) {
      add(*menu);
    }
  }
}

MenuBar::~MenuBar() {
  for (auto *menu: menus_) {
    menu->bar_ = nullptr;
    menu->focusGroup(nullptr);
  }
}

void MenuBar::add(Menu &menu) {
  if (std::ranges::find(menus_, &menu) != menus_.end()) {
    return;
  }
  menus_.push_back(&menu);
  menu.bar_ = this;
  menu.focusGroup(&group_);
}

void MenuBar::arrange(const Rectangle &area, Dim spacing) {
  auto x{area.x1()};
  for (auto *menu: menus_) {
    const auto width{menu->preferredSize().x()};
    menu->move(Rectangle{x, area.y1(), toDim(x + width), toDim(area.y1() + 1)});
    x = toDim(x + width + spacing);
  }
}

auto MenuBar::move(Menu &from, int direction, bool open) -> bool {
  const auto found{std::ranges::find(menus_, &from)};
  if (found == menus_.end()) {
    return false;
  }
  const auto index{static_cast<std::ptrdiff_t>(found - menus_.begin()) + (direction > 0? 1: -1)};
  if (index < 0 or index >= static_cast<std::ptrdiff_t>(menus_.size())) {
    return false;
  }
  auto *target{menus_[static_cast<std::size_t>(index)]};
  if (auto *root{from.root()}; root) {
    root->focus(target);
  }
  if (open) {
    target->open();
  }
  return true;
}

void MenuBar::focusMenu(Menu &menu, bool open) {
  if (auto *root{menu.root()}; root) {
    root->focus(&menu);
  }
  if (open) {
    menu.open();
  }
}

void MenuBar::mnemonic(Menu &menu, Unicode key) {
  menu.mnemonic(key);
}

void MenuBar::accelerate(Unicode key) {
  auto *barRoot{root()};
  if (not barRoot) {
    return;
  }
  actions_.emplace_back();
  auto &action{actions_.back()};
  action.handler(
    [this] {
      auto *barRoot{root()};
      if (not barRoot) {
        return;
      }
      if (auto *focused{barRoot->focus()}; focused and group_.contains(focused)) {
        return; // the bar has the focus already
      }
      if (auto *first{group_.first()}; first) {
        barRoot->focus(first);
      }
    }
  );
  action.shortcuts({key});
  barRoot->add(action);
}

void MenuBar::forget(Menu &menu) {
  std::erase(menus_, &menu);
}

auto MenuBar::root() -> Root * {
  return menus_.empty()? nullptr: menus_.front()->root();
}

} // namespace jwezel::ui
