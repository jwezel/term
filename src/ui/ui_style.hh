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

///
/// How a list box marks its current item
enum class ItemSelectionStyle: u1 {
  caret,    ///< Put the list marker before the current item.
  highlight ///< Highlight the whole current row.
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
/// Where a popup of a choice control opens
///
/// `inside` opens the popup as a widget inside the window, clipped to the content area; it flips above the control
/// when there is not enough room below. `window` opens it in a transient window of its own, above the owner window.
enum class PopupPlacement: u1 {
  inside, ///< Inside the content of the owner window, flipped above the control when needed
  window  ///< In a transient window above the owner window
};

///
/// Style of the user interface, without colors
///
/// The style says what the elements of the UI look like: glyphs, selection indication, and the decoration of windows. It is a plain
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
  Brackets list{" ", " "}; ///< Glyphs surrounding each item in a list box.
  std::string_view listMarker{"🢒"}; ///< Marker shown before the current item in caret mode.
  ItemSelectionStyle itemSelectionStyle{ItemSelectionStyle::highlight}; ///< Default current-item style for list boxes.
  std::string_view labelSuffix{}; ///< Suffix shown after the text of a label that has a buddy.
  PopupPlacement popupPlacement{PopupPlacement::inside}; ///< Where a popup of a choice control opens.
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
/// Color role for a disabled popup item.
inline constexpr std::string_view popupDisabledRole{"popup.disabled"};
/// Color role for a list box.
inline constexpr std::string_view listRole{"list"};
/// Color role for a focused list box.
inline constexpr std::string_view listFocusRole{"list.focus"};
/// Color role for a selected list box item.
inline constexpr std::string_view listSelectedRole{"list.selected"};
/// Color role for the current list box item.
inline constexpr std::string_view listCurrentRole{"list.current"};
/// Color role for the current list box item when it is also selected.
inline constexpr std::string_view listCurrentSelectedRole{"list.current.selected"};
/// Color role for a disabled list box item.
inline constexpr std::string_view listDisabledRole{"list.disabled"};

} // namespace jwezel::ui
