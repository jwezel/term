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
  normal, ///< The widget is idle.
  focus,  ///< The widget receives keyboard input.
  active  ///< The widget is in use, such as a pressed button or open drop-down.
};

/// Number of visual states represented in state-indexed style arrays.
inline constexpr std::size_t WidgetStates{3};

///
/// Glyphs to the left and to the right of what a widget displays
struct Brackets {
  std::string_view left;  ///< Glyphs displayed before the widget's content.
  std::string_view right; ///< Glyphs displayed after the widget's content.
};

///
/// Glyphs by widget state
using BracketsByState = std::array<Brackets, WidgetStates>;

///
/// Style of the user interface, without colors
///
/// The style says what the elements of the UI look like: glyphs, and the decoration of windows. It is a plain
/// value that is meant to be constant. Colors are a matter of the `ColorStyle`, which has a role for every
/// widget and state. The default style looks like this, where the button has the focus
/// when it is in angle brackets:
///
/// @code
/// [ Button ]   < Button >   [ Drop down ▼ ]    ╭─ List ─╮
/// @endcode
struct UiStyle {
  const Decoration *decoration{nullptr}; ///< Window decoration, or `defaultDecoration()` when null; non-owning.
  BracketsByState button{ ///< Button glyphs for normal, focus, and active states, in that order.
    Brackets{"[ ", " ]"}, Brackets{"< ", " >"}, Brackets{"< ", " >"}
  };
  BracketsByState dropDown{ ///< Drop-down glyphs by state; the right glyph includes the arrow.
    Brackets{"[ ", " ▼ ]"}, Brackets{"[ ", " ▼ ]"}, Brackets{"[ ", " ▼ ]"}
  };
  Brackets popup{" ", " "}; ///< Glyphs surrounding each item in a drop-down popup.
};

///
/// Get the UI style that is used where none is given
///
/// All places that take an optional UI style resolve a missing one here, which makes this the one place to
/// change when there is a global style.
auto defaultUiStyle() -> const UiStyle &;

/// Color roles by widget state; a missing state-specific role falls back to the normal role.
/// Button color roles in `WidgetState` order.
inline constexpr std::array<std::string_view, WidgetStates> buttonRoles{"button", "button.focus", "button.active"};
/// Text-input color roles in `WidgetState` order.
inline constexpr std::array<std::string_view, WidgetStates> inputRoles{"input", "input.focus", "input.active"};
/// Drop-down color roles in `WidgetState` order.
inline constexpr std::array<std::string_view, WidgetStates> dropDownRoles{"dropdown", "dropdown.focus", "dropdown.active"};
/// Color role for labels.
inline constexpr std::string_view labelRole{"label"};
/// Color role for a drop-down popup.
inline constexpr std::string_view popupRole{"popup"};
/// Color role for the highlighted popup item.
inline constexpr std::string_view popupHighlightRole{"popup.highlight"};

} // namespace jwezel::ui
