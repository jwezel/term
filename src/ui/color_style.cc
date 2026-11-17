#include "color_style.hh"

namespace jwezel::ui {

namespace {

constexpr CharAttributes NoAttributes{};

// Colors are left to the terminal; the active window is told apart by its bold title
constexpr ColorRoles<6> defaultRoles{
  {"frame", CharAttributes{}},
  {"frame.active", CharAttributes{}},
  {"frame.inactive", CharAttributes{}},
  {"title", CharAttributes{}},
  {"title.active", CharAttributes{RgbNone, RgbNone, bold}},
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
