#include "focus_group.hh"

#include "widget.hh"

#include <algorithm>
#include <iterator>

namespace jwezel::ui {

FocusGroup::FocusGroup(std::vector<Widget *> members) {
  for (auto *member: members) {
    if (member) {
      member->focusGroup(this);
    }
  }
}

FocusGroup::~FocusGroup() {
  for (auto *member: members_) {
    member->focusGroup_ = nullptr;
  }
}

void FocusGroup::add(Widget &member) {
  if (std::ranges::find(members_, &member) == members_.end()) {
    members_.push_back(&member);
  }
}

void FocusGroup::remove(Widget &member) {
  std::erase(members_, &member);
}

auto FocusGroup::contains(const Widget *member) const -> bool {
  return std::ranges::find(members_, member) != members_.end();
}

auto FocusGroup::first() -> Widget * {
  for (auto *member: members_) {
    if (member->tabbable()) {
      return member;
    }
  }
  return nullptr;
}

auto FocusGroup::last() -> Widget * {
  for (auto member = members_.rbegin(); member != members_.rend(); ++member) {
    if ((*member)->tabbable()) {
      return *member;
    }
  }
  return nullptr;
}

auto FocusGroup::next(Widget &current) -> Widget * {
  const auto found{std::ranges::find(members_, &current)};
  if (found == members_.end()) {
    return nullptr;
  }
  for (auto member = found + 1; member != members_.end(); ++member) {
    if ((*member)->tabbable()) {
      return *member;
    }
  }
  return nullptr;
}

auto FocusGroup::previous(Widget &current) -> Widget * {
  const auto found{std::ranges::find(members_, &current)};
  if (found == members_.end()) {
    return nullptr;
  }
  for (auto member = std::make_reverse_iterator(found); member != members_.rend(); ++member) {
    if ((*member)->tabbable()) {
      return *member;
    }
  }
  return nullptr;
}

void FocusGroup::forget(Widget &member) {
  std::erase(members_, &member);
}

} // namespace jwezel::ui
