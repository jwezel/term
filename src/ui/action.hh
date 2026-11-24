#pragma once

#include <term/text.hh>

#include <functional>
#include <initializer_list>
#include <vector>

namespace jwezel::ui {

struct ActionHost;

///
/// Something the user can do, apart from the widget that offers it
///
/// An action has a handler, that is what happens, and shortcuts: keys that trigger it. It does not draw anything
/// and does not need a widget: an action without any widget is a hidden one, like Escape cancelling a dialog or
/// Ctrl-C quitting. Widgets that show something to do, like buttons, trigger actions as well, so a button and a key
/// can be the same action.
///
/// An action is added to hosts, which are windows (a root) and widgets, and which it is removed from again when it
/// is destroyed, like the hosts forget it when they are destroyed. It does not belong to anything: whoever creates it
/// keeps it for as long as it is needed.
///
/// - Added to a window, its shortcuts work in the whole window, when no widget has made use of the key. That is how
///   a window gets a default action (Enter) and a cancel action (Escape).
/// - Added to a widget, its shortcuts work while the widget has the focus.
struct Action {
  /// Callable invoked when an enabled action is triggered.
  using Handler = std::function<void()>;

  ///
  /// Create Action
  ///
  /// @param[in]  handler    What the action does
  /// @param[in]  shortcuts  The keys that trigger it, as they are in a key event: a character, or a `Key`
  explicit Action(const Handler &handler = {}, std::initializer_list<Unicode> shortcuts = {});

  Action(const Action &) = delete;

  Action(Action &&) = delete;

  auto operator=(const Action &) -> Action & = delete;

  auto operator=(Action &&) -> Action & = delete;

  ~Action();

  ///
  /// Do what the action is for, if it is enabled
  ///
  /// @return     True if it was done
  auto trigger() -> bool;

  [[nodiscard]] auto enabled() const -> bool {return enabled_;}

  ///
  /// An action that is not enabled cannot be triggered, by a key or otherwise
  void enabled(bool enabled) {enabled_ = enabled;}

  [[nodiscard]] auto shortcuts() const -> const std::vector<Unicode> & {return shortcuts_;}

  void shortcuts(std::initializer_list<Unicode> shortcuts) {shortcuts_ = shortcuts;}

  ///
  /// Set the shortcuts
  ///
  /// @param[in]  shortcuts  The keys that trigger the action
  void shortcuts(const std::vector<Unicode> &shortcuts) {shortcuts_ = shortcuts;}

  ///
  /// Whether a key is a shortcut of the action
  [[nodiscard]] auto hasShortcut(Unicode key) const -> bool;

  void handler(const Handler &handler) {handler_ = handler;}

  private:
  Handler handler_;
  std::vector<Unicode> shortcuts_;
  bool enabled_{true};
  std::vector<ActionHost *> hosts_; // weak references
  friend struct ActionHost;
};

///
/// Something that actions can be added to
///
/// Hosts and actions know each other, and let go of each other when one of them goes away.
struct ActionHost {
  ActionHost() = default;

  ActionHost(const ActionHost &) = delete;

  ActionHost(ActionHost &&) = delete;

  auto operator=(const ActionHost &) -> ActionHost & = delete;

  auto operator=(ActionHost &&) -> ActionHost & = delete;

  ~ActionHost();

  ///
  /// Add an action, which can be in any number of hosts; nothing happens if it is in this one already
  void addAction(Action &action);

  ///
  /// Remove an action; nothing happens if it is not here
  void removeAction(Action &action);

  [[nodiscard]] auto actions() const -> const std::vector<Action *> & {return actions_;}

  ///
  /// Trigger the first enabled action that has the key as a shortcut
  ///
  /// @return     True if there was one
  auto triggerShortcut(Unicode key) -> bool;

  ///
  /// Trigger all enabled actions, in the order they were added
  ///
  /// @return     True if there was one
  auto triggerActions() -> bool;

  private:
  std::vector<Action *> actions_; // weak references
};

} // namespace jwezel::ui
