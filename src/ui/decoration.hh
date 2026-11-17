#pragma once

#include "color_style.hh"

#include <term/geometry.hh>
#include <term/text.hh>

#include <array>
#include <string_view>

namespace jwezel::ui {

///
/// State of a decorated window or frame
///
/// A decoration can show each state differently. The states are also what the color roles of a decoration are
/// named after, see `frameRole` and `titleRole`.
enum class DecorationState: u1 {
  inactive, ///< The window is not the one that gets the input
  active    ///< The window gets the keyboard input
};

///
/// Get the name of the color role of the frame in a state
[[nodiscard]] constexpr auto frameRole(DecorationState state) -> std::string_view {
  switch (state) {
    case DecorationState::active:
    return "frame.active";

    case DecorationState::inactive:
    break;
  }
  return "frame.inactive";
}

///
/// Get the name of the color role of the title in a state
[[nodiscard]] constexpr auto titleRole(DecorationState state) -> std::string_view {
  switch (state) {
    case DecorationState::active:
    return "title.active";

    case DecorationState::inactive:
    break;
  }
  return "title.inactive";
}

///
/// Policy for the decoration of a window or frame
///
/// A decoration decides what the frame around the content of a window looks like: the lines, the title and
/// whatever else a style comes up with, in every state. It is a strategy that does not entail colors: those come
/// from a `ColorStyle`, which the decoration only asks for the colors of its roles.
///
/// Decorations are stateless and constant. A window refers to its decoration, which must outlive it; windows
/// without a decoration use `defaultDecoration()`.
struct Decoration {
  Decoration() = default;

  Decoration(const Decoration &) = delete;

  Decoration(Decoration &&) = delete;

  auto operator=(const Decoration &) -> Decoration & = delete;

  auto operator=(Decoration &&) -> Decoration & = delete;

  virtual ~Decoration() = default;

  ///
  /// Get the width of the frame
  ///
  /// The content of a decorated window has to keep clear of this much on all sides.
  [[nodiscard]] virtual auto inset() const -> Dim = 0;

  ///
  /// Draw the decoration
  ///
  /// The result is as large as the window. Cells of the result that are `Untouched` leave what is in the window
  /// there as it is, all others replace it.
  ///
  /// @param[in]  size        The size of the window
  /// @param[in]  state       The state
  /// @param[in]  title       The title (only the first line is used)
  /// @param[in]  background  The background of the window
  /// @param[in]  colors      The color style
  ///
  /// @return     The decoration
  [[nodiscard]] virtual auto draw(
    const Vector &size,
    DecorationState state,
    const Text &title,
    const Char &background,
    const ColorStyle &colors
  ) const -> Text = 0;
};

///
/// A cell that a decoration does not draw
inline const Char Untouched{NoneRune, RgbNone, RgbNone, {}, AttributeMode::merge};

///
/// Style of the lines of a border
struct BorderStyle {
  u1 strength{1};       ///< 1 for a light line, 2 for a heavy one
  u1 dash{0};           ///< 0 for a solid line, anything else for a dashed one
  bool rounded{false};  ///< Rounded corners (only for light lines)
};

///
/// Decoration with a box around the window and the title in its top line
///
/// The border can be drawn differently in each state. By default, the active window has a solid light border,
/// and the inactive one a dashed light border.
struct BoxDecoration: Decoration {
  ///
  /// Create BoxDecoration
  ///
  /// @param[in]  active          Border style of the active state
  /// @param[in]  inactive        Border style of the inactive state
  /// @param[in]  titleAlignment  Where the title is in the top line
  explicit BoxDecoration(
    const BorderStyle &active = BorderStyle{},
    const BorderStyle &inactive = BorderStyle{1, 1, false},
    Alignment titleAlignment = Alignment::left
  );

  [[nodiscard]] auto inset() const -> Dim override {return 1;}

  [[nodiscard]] auto draw(
    const Vector &size,
    DecorationState state,
    const Text &title,
    const Char &background,
    const ColorStyle &colors
  ) const -> Text override;

  private:
  std::array<BorderStyle, 2> borders_; // by state
  Alignment titleAlignment_;
};

///
/// Get the decoration that is used where none is given
///
/// All places that take an optional decoration resolve a missing one here, which makes this the one place to
/// change when there is a global style.
auto defaultDecoration() -> const Decoration &;

} // namespace jwezel::ui
