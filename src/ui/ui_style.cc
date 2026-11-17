#include "ui_style.hh"

namespace jwezel::ui {

namespace {

constexpr UiStyle defaultStyle{};

} // namespace

auto defaultUiStyle() -> const UiStyle & {
  return defaultStyle;
}

} // namespace jwezel::ui
