#include "color_style.hh"

namespace jwezel::ui {

namespace {

constexpr CharAttributes NoAttributes{};

// The default style is neutral: colors and attributes are left to the terminal, except for what is needed to see
// anything: the item of a list that is highlighted, and the widget that gets the keyboard input, which is tinted.
// The roles are listed to show what a style can say something about; the states of a window are told apart by the
// style of its border.
//
// The tint is mixed into the background that is there, so it lightens the widget of a window with colors, and is a
// plain gray on the background of the terminal, on which any default text is readable. A pressed button and an open
// drop down are the focused widget, so they have it as well.
constexpr CharAttributes focusTint{RgbNone, RgbGray5, {}, AttributeMode::mix};

constexpr ColorRoles<24> defaultRoles{
  {"frame", CharAttributes{}},
  {"frame.active", CharAttributes{}},
  {"frame.inactive", CharAttributes{}},
  {"title", CharAttributes{}},
  {"title.active", CharAttributes{}},
  {"title.inactive", CharAttributes{}},
  {"label", CharAttributes{}},
  {"button", CharAttributes{}},
  {"button.focus", focusTint},
  {"button.active", focusTint},
  {"input", CharAttributes{}},
  {"input.focus", focusTint},
  {"dropdown", CharAttributes{}},
  {"dropdown.focus", focusTint},
  {"dropdown.active", focusTint},
  {"popup", CharAttributes{}},
  {"popup.highlight", CharAttributes{RgbNone, RgbNone, reverse}},
  {"popup.disabled", CharAttributes{RgbGray5}},
  {"list", CharAttributes{}},
  {"list.focus", focusTint},
  {"list.selected", CharAttributes{RgbNone, RgbNone, reverse}},
  {"list.current", CharAttributes{RgbNone, RgbNone, reverse}},
  {"list.current.selected", CharAttributes{RgbNone, RgbNone, static_cast<Attributes>(bold | reverse)}},
  {"list.disabled", CharAttributes{RgbGray5}},
};

constexpr ColorStyle defaultStyle{defaultRoles};

} // namespace

auto ColorStyle::find(std::string_view role) const -> const CharAttributes * {
  while (true) {
    if (const auto *attributes{find_(roles_, role)}; attributes) {
      return attributes;
    }
    const auto dot{role.rfind('.')};
    if (dot == std::string_view::npos) {
      return nullptr;
    }
    role = role.substr(0, dot);
  }
}

auto ColorStyle::operator [](std::string_view role) const -> const CharAttributes & {
  const auto *attributes{find(role)};
  return attributes? *attributes: NoAttributes;
}

auto defaultColorStyle() -> const ColorStyle & {
  return defaultStyle;
}

} // namespace jwezel::ui
