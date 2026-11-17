#pragma once

#include <term/text.hh>

#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include <cstddef>
#include <string_view>

namespace jwezel::ui {

///
/// Table of color roles
///
/// A color style is a constant table from role names to attributes, so it costs nothing at run time and can be
/// built at compile time:
///
/// @code
/// constexpr ColorRoles<2> myRoles{
///   {"title", CharAttributes{RgbYellow}},
///   {"title.inactive", CharAttributes{RgbGray5}}
/// };
/// @endcode
///
/// The attributes of a role are meant to be merged into what is there (see `AttributeMode::merge`): what a role
/// does not specify, like the background, shines through.
///
/// @tparam N     The number of roles
template<std::size_t N>
using ColorRoles = frozen::unordered_map<frozen::string, CharAttributes, N>;

///
/// A set of colors by role
///
/// This is a view of a `ColorRoles` table, which must have static storage duration (declare it `constexpr` at
/// namespace scope): the style refers to the table and never owns it, like all styles. Styles are cheap values
/// that can be copied and passed around.
///
/// Roles are named like `frame.inactive`. If a role is not in the table, the role without its last
/// component is tried (`frame`), so a table only has to list what is different, and nothing at all
/// is the same as no attributes.
struct ColorStyle {
  ///
  /// Create ColorStyle
  ///
  /// @param[in]  roles  The table, which must outlive the style
  template<std::size_t N>
  constexpr ColorStyle(const ColorRoles<N> &roles) noexcept: // NOLINT(google-explicit-constructor): a table is a style
  roles_{&roles},
  find_{&findIn<N>}
  {}

  ///
  /// Find a role
  ///
  /// @param[in]  role  The role name
  ///
  /// @return     The attributes of the role or of the closest enclosing one, nullptr if there is none
  [[nodiscard]] auto find(std::string_view role) const -> const CharAttributes *;

  ///
  /// Get the attributes of a role
  ///
  /// @param[in]  role  The role name
  ///
  /// @return     The attributes of the role or of the closest enclosing one, no attributes if there is none
  [[nodiscard]] auto operator [](std::string_view role) const -> const CharAttributes &;

  private:
  template<std::size_t N>
  static auto findIn(const void *roles, std::string_view role) -> const CharAttributes * {
    const auto &table{*static_cast<const ColorRoles<N> *>(roles)};
    const auto found{table.find(frozen::string{role})};
    return found == table.end()? nullptr: &found->second;
  }

  const void *roles_;
  const CharAttributes *(*find_)(const void *, std::string_view);
};

///
/// Get the color style that is used where none is given
///
/// All places that take an optional color style resolve a missing one here, which makes this the one place to
/// change when there is a global style.
auto defaultColorStyle() -> const ColorStyle &;

} // namespace jwezel::ui
