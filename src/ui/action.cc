#include "action.hh"

#include <algorithm>

namespace jwezel::ui {

Action::Action(const Handler &handler, std::initializer_list<Unicode> shortcuts):
handler_{handler},
shortcuts_{shortcuts}
{}

Action::~Action() {
  // removeAction changes the list, so work on a copy
  for (auto *host: std::vector<ActionHost *>{hosts_}) {
    host->removeAction(*this);
  }
}

auto Action::trigger() -> bool {
  if (!enabled_ or !handler_) {
    return false;
  }
  // The handler may well remove the action, or its host: it is not touched after that
  const auto handler{handler_};
  handler();
  return true;
}

auto Action::hasShortcut(Unicode key) const -> bool {
  return std::ranges::find(shortcuts_, key) != shortcuts_.end();
}

ActionHost::~ActionHost() {
  for (auto *action: actions_) {
    std::erase(action->hosts_, this);
  }
}

void ActionHost::addAction(Action &action) {
  if (std::ranges::find(actions_, &action) == actions_.end()) {
    actions_.push_back(&action);
    action.hosts_.push_back(this);
  }
}

void ActionHost::removeAction(Action &action) {
  std::erase(actions_, &action);
  std::erase(action.hosts_, this);
}

auto ActionHost::triggerShortcut(Unicode key) -> bool {
  // The handler may change the list: look up the action first, and then do not rely on the list anymore
  const auto found{
    std::ranges::find_if(actions_, [key](const Action *action) {return action->enabled() and action->hasShortcut(key);})
  };
  if (found == actions_.end()) {
    return false;
  }
  return (*found)->trigger();
}

auto ActionHost::triggerActions() -> bool {
  auto done{false};
  for (auto *action: std::vector<Action *>{actions_}) { // a copy, handlers may change the list
    if (std::ranges::find(actions_, action) != actions_.end()) { // not removed by a handler of another action
      done = action->trigger() or done;
    }
  }
  return done;
}

} // namespace jwezel::ui
