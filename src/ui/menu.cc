#include "menu.hh"

#include "menu_bar.hh"

#include <term/event.hh>
#include <term/keyboard.hh>

#include <algorithm>
#include <string>
#include <vector>

namespace jwezel::ui {

using std::string, std::max, std::vector;

namespace {

// The text of a menu row, without the line ends that the Text conversion adds
auto rowText(const Text &text) -> string {
  auto result{string(text)};
  while (not result.empty() and result.back() == '\n') {
    result.pop_back();
  }
  return result;
}

// What a key is called in a menu hint
auto shortcutLabel(Unicode key) -> string {
  if (key >= 1 and key <= 26) {
    return string("Ctrl-") + static_cast<char>('A' + key - 1);
  }
  if (const auto name{nameOfKey(key)}; name) {
    if (*name == "Escape") {
      return "Esc";
    }
    return string(*name);
  }
  if (key < 128) {
    return string(1, static_cast<char>(key));
  }
  return {};
}

// The hint that an item shows when it has none of its own: its action's first shortcut
auto hintOf(const MenuItem &item) -> Text {
  if (item.hint.width() > 0 or not item.action or item.action->shortcuts().empty()) {
    return item.hint;
  }
  return Text{shortcutLabel(item.action->shortcuts().front())};
}

} // namespace

MenuItem MenuItem::rule() {
  MenuItem item;
  item.separator = true;
  return item;
}

MenuItem MenuItem::withSubmenu(const Text &text, Menu &submenu) {
  MenuItem item;
  item.text = text;
  item.submenu = &submenu;
  return item;
}

Menu::Menu(Container &parent, const Rectangle &area, const std::vector<MenuItem> &items, const Char &background):
PopupChoice{parent, area, background},
items_{items}
{
  rebuild();
  hostShortcuts();
  render();
  layoutChanged();
}

Menu::Menu(Container &parent, const std::vector<MenuItem> &items, const Char &background):
PopupChoice{parent, background},
items_{items}
{
  rebuild();
  hostShortcuts();
  render();
  layoutChanged();
}

Menu::~Menu() {
  if (bar_) {
    bar_->forget(*this);
  }
}

void Menu::items(const std::vector<MenuItem> &items) {
  close();
  clearShortcuts();
  items_ = items;
  rebuild();
  hostShortcuts();
  render();
  layoutChanged();
}

auto Menu::checked(ListIndex index) const -> bool {
  return items_[static_cast<std::size_t>(index)].checked;
}

void Menu::checked(ListIndex index, bool checked) {
  auto &item{items_[static_cast<std::size_t>(index)]};
  if (item.checked == checked) {
    return;
  }
  item.checked = checked;
  rebuild();
  render();
  if (isOpen()) {
    close();
  }
}

void Menu::mnemonic(Unicode key) {
  mnemonicAction_.handler(
    [this] {
      if (auto *root{this->root()}; root) {
        root->focus(this);
        open();
      }
    }
  );
  mnemonicAction_.shortcuts({key});
  if (auto *root{this->root()}; root) {
    root->add(mnemonicAction_);
  }
}

auto Menu::preferredSize() -> Vector {
  // The closed control shows what the menu is called, not the width of its items
  const auto &glyphs{closedGlyphs(static_cast<std::size_t>(WidgetState::normal))};
  const auto padding{toDim(Text{glyphs.left}.width() + Text{glyphs.right}.width())};
  return Vector{toDim(caption_.width() + padding), 1};
}

auto Menu::popupItems() -> ListSource<ListIndex> {
  return ListSource<ListIndex>{
    [this] {return static_cast<ListIndex>(rows_.size());},
    [this](ListIndex index) {return rows_[static_cast<std::size_t>(index)];}
  };
}

auto Menu::popupItemEnabled(ListIndex index) const -> bool {
  const auto &item{items_[static_cast<std::size_t>(index)]};
  if (item.separator) {
    return false;
  }
  if (item.submenu) {
    return true;
  }
  const auto *action{item.action};
  return action and action->enabled();
}

void Menu::popupActivate(ListIndex index, const Event & /*event*/) {
  auto &item{items_[static_cast<std::size_t>(index)]};
  if (item.submenu) {
    openChild(*item.submenu, index);
    return;
  }
  auto *action{item.action};
  // Choosing an item ends the whole path of menus that led to it
  auto *top{this};
  while (top->parentMenu_) {
    top = top->parentMenu_;
  }
  top->close();
  if (action) {
    action->trigger();
  }
}

auto Menu::closedText() const -> const Text * {
  return &caption_;
}

auto Menu::handleKey(const Event &event) -> bool {
  const auto key{event.as<KeyEvent>().key()};
  if (child_) {
    if (key == Key::Escape or key == Key::Left) {
      closeChild();
      return true;
    }
    // The open submenu has the input; the parent does not take it when the submenu does not
    return child_->handleKey(event);
  }
  if (key == Key::Right or key == Key::Left) {
    const auto direction{key == Key::Right? 1: -1};
    if (key == Key::Right and isOpen()) {
      const auto index{highlighted()};
      if (index >= 0 and items_[static_cast<std::size_t>(index)].submenu) {
        openChild(*items_[static_cast<std::size_t>(index)].submenu, index);
        return true;
      }
    }
    if (bar_) {
      return bar_->move(*this, direction, isOpen());
    }
    return false;
  }
  return PopupChoice::handleKey(event);
}

void Menu::close() {
  closeChild();
  PopupChoice::close();
}

void Menu::rebuild() {
  Dim textWidth{0}, hintWidth{0};
  for (const auto &item: items_) {
    if (not item.separator) {
      textWidth = max(textWidth, item.text.width());
    }
    if (item.submenu) {
      hintWidth = max(hintWidth, Text{uiStyle().menuSubmenu}.width());
    } else {
      hintWidth = max(hintWidth, hintOf(item).width());
    }
  }
  const auto checkWidth{Text{uiStyle().menuCheck}.width()};
  const auto total{toDim(checkWidth + 1 + textWidth + (hintWidth > 0? 1 + hintWidth: 0))};
  rows_.clear();
  rows_.reserve(items_.size());
  const string check{uiStyle().menuCheck}, submenu{uiStyle().menuSubmenu};
  for (const auto &item: items_) {
    if (item.separator) {
      string rule;
      rule.reserve(static_cast<std::size_t>(total) * 3); // an en dash is three bytes in UTF-8
      for (Dim column = 0; column < total; ++column) {
        rule += "─";
      }
      rows_.emplace_back(rule);
      continue;
    }
    string row{item.checked? check + " ": string(static_cast<std::size_t>(checkWidth + 1), ' ')};
    row += rowText(item.text);
    row += string(static_cast<std::size_t>(textWidth - item.text.width()), ' ');
    if (item.submenu) {
      const auto width{Text{submenu}.width()};
      row += string(static_cast<std::size_t>(1 + hintWidth - width), ' ');
      row += submenu;
    } else if (const auto hint{hintOf(item)}; hint.width() > 0) {
      row += string(static_cast<std::size_t>(1 + hintWidth - hint.width()), ' ');
      row += rowText(hint);
    }
    rows_.emplace_back(row);
  }
}

void Menu::hostShortcuts() {
  auto *root{this->root()};
  if (not root) {
    return;
  }
  for (const auto &item: items_) {
    if (not item.action or item.action->shortcuts().empty()) {
      continue;
    }
    shortcutActions_.emplace_back();
    auto &action{shortcutActions_.back()};
    auto *target{item.action};
    action.handler(
      [this, target] {
        // An accelerator ends the menus that are open, then does what the item does
        auto *top{this};
        while (top->parentMenu_) {
          top = top->parentMenu_;
        }
        top->close();
        if (target) {
          target->trigger();
        }
      }
    );
    action.shortcuts(item.action->shortcuts());
    root->add(action);
  }
}

void Menu::clearShortcuts() {
  shortcutActions_.clear(); // the actions take themselves out of the window
}

void Menu::openChild(Menu &submenu, ListIndex index) {
  if (not isOpen()) {
    return;
  }
  closeChild();
  submenu.parentMenu_ = this;
  child_ = &submenu;
  const auto row{popupItemArea(index)};
  submenu.openAt(Rectangle{row.x2(), row.y1(), row.x2(), row.y1()}); // to the right of the item, at its row
  if (not submenu.isOpen()) {
    child_ = nullptr;
    submenu.parentMenu_ = nullptr;
  }
}

void Menu::closeChild() {
  if (not child_) {
    return;
  }
  auto *child{child_};
  child_ = nullptr;
  child->parentMenu_ = nullptr;
  child->close();
}

} // namespace jwezel::ui
