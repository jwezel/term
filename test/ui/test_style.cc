#include <ui/color_style.hh>

#include <term/text.hh>

#include <doctest/doctest.h>

using
  jwezel::AttributeMode,
  jwezel::bold,
  jwezel::CharAttributes,
  jwezel::reverse,
  jwezel::Rgb,
  jwezel::RgbBlue,
  jwezel::RgbNone,
  jwezel::RgbRed,
  jwezel::RgbYellow,
  jwezel::ui::ColorRoles,
  jwezel::ui::ColorStyle,
  jwezel::ui::defaultColorStyle;

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

namespace {

// The tables are constant expressions, and so are the styles that refer to them
constexpr ColorRoles<4> testRoles{
  {"frame", CharAttributes{RgbBlue}},
  {"frame.inactive", CharAttributes{RgbNone, RgbBlue}},
  {"title", CharAttributes{RgbYellow, RgbNone, bold, AttributeMode::merge}},
  {"title.active.focus", CharAttributes{RgbRed}},
};
constexpr ColorStyle testStyle{testRoles};

static_assert(testRoles.find("frame") != testRoles.end());
static_assert(testRoles.find("frame")->second.fg == RgbBlue);

auto fg(const ColorStyle &style, std::string_view role) -> Rgb {
  return style[role].fg;
}

} // namespace

TEST_CASE("Color style") {
  SUBCASE("Exact role") {
    CHECK_EQ(testStyle.find("frame")->fg, RgbBlue);
    CHECK_EQ(testStyle.find("frame.inactive")->bg, RgbBlue);
    CHECK_EQ(testStyle.find("title")->attr, bold);
    CHECK_EQ(testStyle.find("title")->mix, AttributeMode::merge);
  }
  SUBCASE("The most specific role wins") {
    CHECK_EQ(testStyle.find("frame.inactive")->fg, RgbNone); // not the fg of "frame"
    CHECK_EQ(fg(testStyle, "frame"), RgbBlue);
  }
  SUBCASE("A missing role falls back to the enclosing one") {
    CHECK_EQ(testStyle.find("frame.active"), testStyle.find("frame"));
    CHECK_EQ(testStyle.find("title.inactive"), testStyle.find("title"));
    CHECK_EQ(testStyle.find("title.active.focus.deep"), testStyle.find("title.active.focus")); // several levels
    CHECK_EQ(testStyle.find("frame.inactive.focus"), testStyle.find("frame.inactive"));
    CHECK_EQ(fg(testStyle, "title.active"), RgbYellow); // "title.active.focus" is more specific than this
  }
  SUBCASE("A role is not a prefix") {
    CHECK_EQ(testStyle.find("frames"), nullptr);
    CHECK_EQ(testStyle.find("fram"), nullptr);
    CHECK_EQ(testStyle.find("titlebar.active"), nullptr);
  }
  SUBCASE("Nothing") {
    CHECK_EQ(testStyle.find("button"), nullptr);
    CHECK_EQ(testStyle.find(""), nullptr);
    CHECK_EQ(testStyle.find("."), nullptr);
    CHECK_EQ(testStyle["button"], CharAttributes{}); // no attributes at all
    CHECK_EQ(testStyle["button.focus"], CharAttributes{});
  }
  SUBCASE("A style is a value") {
    const ColorStyle copy{testStyle};
    CHECK_EQ(copy.find("frame"), testStyle.find("frame"));
    ColorStyle other{defaultColorStyle()};
    CHECK_EQ(other.find("frame")->fg, RgbNone);
    other = testStyle; // and it can be switched
    CHECK_EQ(other.find("frame")->fg, RgbBlue);
  }
  SUBCASE("Styles do not see each other's roles") {
    static constexpr ColorRoles<1> roles{{"button", CharAttributes{RgbRed}}};
    static constexpr ColorStyle buttons{roles}; // a different table size, the same type
    CHECK_EQ(buttons.find("button")->fg, RgbRed);
    CHECK_EQ(buttons.find("frame"), nullptr);
    CHECK_EQ(testStyle.find("button"), nullptr);
  }
  SUBCASE("Default style") {
    const auto &style{defaultColorStyle()};
    CHECK_EQ(&style, &defaultColorStyle());
    CHECK_EQ(style["title.active"].attr, bold);
    CHECK_EQ(style["title.inactive"], CharAttributes{});
    CHECK_EQ(style["frame.active"], CharAttributes{});
    CHECK_EQ(style["frame.inactive"], CharAttributes{});
    CHECK_EQ(style["nonsense"], CharAttributes{});
    CHECK_NE(style.find("title"), nullptr);
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(readability-magic-numbers)
