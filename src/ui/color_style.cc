#include "color_style.hh"

namespace jwezel::ui {

namespace {

constexpr CharAttributes NoAttributes{};

// The default style is neutral: colors and attributes are left to the terminal. The roles are listed to show what
// a style can say something about; the states of a window are told apart by the style of its border.
constexpr ColorRoles<6> defaultRoles{
  {"frame", CharAttributes{}},
  {"frame.active", CharAttributes{}},
  {"frame.inactive", CharAttributes{}},
  {"title", CharAttributes{}},
  {"title.active", CharAttributes{}},
  {"title.inactive", CharAttributes{}},
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
