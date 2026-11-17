#pragma once

#include "decoration.hh"

#include <term/text.hh>

#include <array>
#include <cstddef>
#include <string_view>

namespace jwezel::ui {

///
/// State of a widget, which widgets can show differently
enum class WidgetState: u1 {
  normal, ///< Nothing special
  focus,  ///< The widget gets the keyboard input
  active  ///< The widget is in use: a button that is pressed, a drop down that is open
};

inline constexpr std::size_t WidgetStates{3};

///
/// Glyphs to the left and to the right of what a widget displays
struct Brackets {
  std::string_view left;
  std::string_view right;
};

///
/// Glyphs by widget state
using BracketsByState = std::array<Brackets, WidgetStates>;

///
/// Style of the user interface, without colors
///
/// The style says what the elements of the UI look like: glyphs, and the decoration of windows. It is a plain
/// value that is meant to be constant. Colors are a matter of the `ColorStyle`, which has a role for every
/// widget and state. The default style looks like this:
///
/// @code
/// [ Button ]   [ Drop down ▼ ]    ╭─ List ─╮
/// @endcode
struct UiStyle {
  const Decoration *decoration{nullptr}; ///< Decoration of windows, nullptr for `defaultDecoration()` (weak reference)
  BracketsByState button{
    Brackets{"[ ", " ]"}, Brackets{"[ ", " ]"}, Brackets{"[ ", " ]"}
  };
  BracketsByState dropDown{ ///< The right glyph includes the arrow
    Brackets{"[ ", " ▼ ]"}, Brackets{"[ ", " ▼ ]"}, Brackets{"[ ", " ▼ ]"}
  };
  Brackets popup{" ", " "}; ///< Left and right of every item of a drop down list
};

///
/// Get the UI style that is used where none is given
///
/// All places that take an optional UI style resolve a missing one here, which makes this the one place to
/// change when there is a global style.
auto defaultUiStyle() -> const UiStyle &;

/// Color roles of the widgets, by state: the roles of a state fall back to the one of the normal state
inline constexpr std::array<std::string_view, WidgetStates> buttonRoles{"button", "button.focus", "button.active"};
inline constexpr std::array<std::string_view, WidgetStates> inputRoles{"input", "input.focus", "input.active"};
inline constexpr std::array<std::string_view, WidgetStates> dropDownRoles{"dropdown", "dropdown.focus", "dropdown.active"};
inline constexpr std::string_view labelRole{"label"};
inline constexpr std::string_view popupRole{"popup"};
inline constexpr std::string_view popupHighlightRole{"popup.highlight"};

} // namespace jwezel::ui
