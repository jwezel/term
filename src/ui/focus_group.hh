#pragma once

#include <vector>

namespace jwezel::ui {

struct Widget;

///
/// A set of widgets that takes one stop in the focus order
///
/// Widgets in a group are still focusable one by one, but Tab enters the group at its first (or, going backwards,
/// its last) member and leaves it again as a whole. Left and Right move between the members while a member has the
/// focus. A menu bar uses a group for its menus, so that the whole bar is one Tab stop.
///
/// The members are weak references: a widget that is destroyed leaves the group, and destroying the group lets go of
/// the widgets it still has. A widget can be in one group at most.
struct FocusGroup {
  FocusGroup() = default;

  ///
  /// Create a group with members
  ///
  /// @param      members  The members, in order; each is moved into the group's focus
  explicit FocusGroup(std::vector<Widget *> members);

  FocusGroup(const FocusGroup &) = delete;

  FocusGroup(FocusGroup &&) = delete;

  auto operator=(const FocusGroup &) -> FocusGroup & = delete;

  auto operator=(FocusGroup &&) -> FocusGroup & = delete;

  ~FocusGroup();

  ///
  /// Add a member, after the ones that are there; it does not get the focus
  ///
  /// @param      member  The member
  void add(Widget &member);

  ///
  /// Remove a member; nothing happens if it is not a member
  ///
  /// @param      member  The member
  void remove(Widget &member);

  [[nodiscard]] auto contains(const Widget *member) const -> bool;

  ///
  /// Get the members, in order
  [[nodiscard]] auto members() const -> const std::vector<Widget *> & {return members_;}

  ///
  /// Get the first member that can have the focus
  ///
  /// @return     The member, or nullptr if there is none
  [[nodiscard]] auto first() -> Widget *;

  ///
  /// Get the last member that can have the focus
  ///
  /// @return     The member, or nullptr if there is none
  [[nodiscard]] auto last() -> Widget *;

  ///
  /// Get the member after one
  ///
  /// @param      current  The current member
  ///
  /// @return     The next member that can have the focus, or nullptr at the end
  [[nodiscard]] auto next(Widget &current) -> Widget *;

  ///
  /// Get the member before one
  ///
  /// @param      current  The current member
  ///
  /// @return     The previous member that can have the focus, or nullptr at the beginning
  [[nodiscard]] auto previous(Widget &current) -> Widget *;

  private:
  // The widget is going away: forget it without touching it
  void forget(Widget &member);

  std::vector<Widget *> members_; // weak references
  friend struct Widget;
};

} // namespace jwezel::ui
