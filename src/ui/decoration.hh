#pragma once

#include "color_style.hh"

#include <term/geometry.hh>
#include <term/text.hh>

#include <array>
#include <string_view>

namespace jwezel::ui {

using Insets = jwezel::Insets; ///< How much of a window's frame its content has to keep clear of, on each side

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
  /// Get the insets of the decoration, from the edges of the window
  ///
  /// The content of a decorated window has to keep clear of the frame on all sides that the decoration draws, in
  /// which the title of a title line is the top line.
  ///
  /// @return     The insets, per side
  [[nodiscard]] virtual auto insets() const -> Insets = 0;

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

auto defaultDecoration() -> const Decoration &;

///
/// How a window or frame is decorated, and how much room its content has
///
/// A framing is the whole of what a decoration means for a window: the decoration itself, the title it shows in
/// its state, and the colors it asks for. Both a window and a frame hold one, so that what they frame and how they
/// frame it cannot disagree.
struct Framing {
  ///
  /// Create a framing
  ///
  /// @param[in]  decoration  The decoration, or `nullptr` for the default one
  /// @param[in]  title       The title (only the first line is used)
  /// @param[in]  state       The state of the decoration
  /// @param[in]  colors      The color style, or `nullptr` for the default one
  Framing(const Decoration *decoration=nullptr, const Text &title={}, DecorationState state=DecorationState::inactive, const ColorStyle *colors=nullptr):
  decoration(decoration),
  title(title),
  state(state),
  colors(colors)
  {}

  ///
  /// Get the insets of the framing: how much the content of the window has to keep clear of its frame
  ///
  /// @return     The insets, per side
  [[nodiscard]] auto insets() const -> Insets {
    return (decoration? *decoration: defaultDecoration()).insets();
  }

  ///
  /// Draw the frame of a window of the given size
  ///
  /// @param[in]  size  The size of the window
  /// @param[in]  background  The background of the window
  ///
  /// @return     The frame
  [[nodiscard]] auto draw(const Vector &size, const Char &background) const -> Text {
    return (decoration? *decoration: defaultDecoration()).draw(size, state, title, background, colors? *colors: defaultColorStyle());
  }

  const Decoration *decoration; // weak reference, nullptr for the default one
  Text title;                     // the title, only the first line is used
  DecorationState state;         // the state the decoration draws in
  const ColorStyle *colors;      // weak reference, nullptr for the default one
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

  [[nodiscard]] auto insets() const -> Insets override;

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
/// Decoration with a title line and no frame
///
/// The content of the window has to keep clear of the title line only, which the insets say, and the title is
/// drawn in the top line of the window.
struct TitleLineDecoration: Decoration {
  ///
  /// Create TitleLineDecoration
  ///
  /// The frame is the title line, and the content starts below it.
  TitleLineDecoration() = default;

  [[nodiscard]] auto insets() const -> Insets override {return Insets{0, 1, 0, 0};}

  [[nodiscard]] auto draw(
    const Vector &size,
    DecorationState state,
    const Text &title,
    const Char &background,
    const ColorStyle &colors
  ) const -> Text override;
};

///
/// Decoration without any frame
///
/// The content of the window fills it entirely, which the insets say.
struct NoDecoration: Decoration {
  ///
  /// Create NoDecoration
  NoDecoration() = default;

  [[nodiscard]] auto insets() const -> Insets override {return Insets{0, 0, 0, 0};}

  [[nodiscard]] auto draw(
    const Vector &size,
    DecorationState /*state*/,
    const Text & /*title*/,
    const Char & /*background*/,
    const ColorStyle & /*colors*/
  ) const -> Text override;
};

} // namespace jwezel::ui
