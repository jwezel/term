#include <ui/color_style.hh>
#include <ui/decoration.hh>
#include <ui/ui_style.hh>

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
  jwezel::ui::BorderStyle,
  jwezel::ui::Brackets,
  jwezel::ui::UiStyle,
  jwezel::ui::WidgetState,
  jwezel::ui::defaultUiStyle,
  jwezel::ui::BoxDecoration,
  jwezel::ui::ColorRoles,
  jwezel::ui::ColorStyle,
  jwezel::ui::DecorationState,
  jwezel::ui::defaultColorStyle,
  jwezel::ui::defaultDecoration,
  jwezel::ui::Untouched,
  jwezel::Alignment,
  jwezel::Char,
  jwezel::NoneRune,
  jwezel::RgbGray5,
  jwezel::Text,
  jwezel::Vector,
  jwezel::underline;

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

// A picture of text, with ~ for the cells a decoration leaves alone
auto picture(const Text &text) -> std::string {
  std::string result;
  for (const auto &line: text.data) {
    for (const auto &character: line) {
      result += character.rune == NoneRune? "~": character.utf8();
    }
    result += "\n";
  }
  return result;
}

constexpr ColorRoles<4> paintRoles{
  {"frame", CharAttributes{RgbBlue}},
  {"frame.inactive", CharAttributes{RgbGray5}},
  {"title", CharAttributes{RgbYellow, RgbNone, bold}},
  {"title.inactive", CharAttributes{RgbGray5, RgbNone, underline}},
};
constexpr ColorStyle paint{paintRoles};

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
    CHECK_EQ(style["title.active"], CharAttributes{}); // neutral
    CHECK_EQ(style["title.inactive"], CharAttributes{});
    CHECK_EQ(style["frame.active"], CharAttributes{});
    CHECK_EQ(style["frame.inactive"], CharAttributes{});
    CHECK_EQ(style["nonsense"], CharAttributes{});
    CHECK_NE(style.find("title"), nullptr);
  }
}

TEST_CASE("Box decoration") {
  const BoxDecoration box;
  const Char blank{' '};
  const auto draw{
    [&](const BoxDecoration &decoration, Vector size, DecorationState state, const Text &title) {
      return picture(decoration.draw(size, state, title, blank, defaultColorStyle()));
    }
  };
  using enum DecorationState;
  SUBCASE("Inset") {
    CHECK_EQ(box.insets(), jwezel::ui::Insets{1, 1, 1, 1});
  }
  SUBCASE("The default is the default decoration") {
    CHECK_EQ(&defaultDecoration(), &defaultDecoration());
    CHECK_EQ(defaultDecoration().insets(), jwezel::ui::Insets{1, 1, 1, 1});
    CHECK_EQ(
      picture(defaultDecoration().draw(Vector{6, 3}, active, Text{"A"}, blank, defaultColorStyle())),
      draw(box, Vector{6, 3}, active, Text{"A"})
    );
  }
  SUBCASE("The states have their own border") {
    CHECK_EQ(draw(box, Vector{8, 4}, active, Text{"Hi"}), "┌Hi────┐\n│~~~~~~│\n│~~~~~~│\n└──────┘\n");
    CHECK_EQ(draw(box, Vector{8, 4}, inactive, Text{"Hi"}), "┌Hi╌╌╌╌┐\n╎~~~~~~╎\n╎~~~~~~╎\n└╌╌╌╌╌╌┘\n");
    const BoxDecoration custom{BorderStyle{2, 0, false}, BorderStyle{1, 0, true}};
    CHECK_EQ(draw(custom, Vector{8, 4}, active, Text{"Hi"}), "┏Hi━━━━┓\n┃~~~~~~┃\n┃~~~~~~┃\n┗━━━━━━┛\n");
    CHECK_EQ(draw(custom, Vector{8, 4}, inactive, Text{"Hi"}), "╭Hi────╮\n│~~~~~~│\n│~~~~~~│\n╰──────╯\n");
  }
  SUBCASE("Title") {
    CHECK_EQ(draw(box, Vector{10, 3}, active, Text{}), "┌────────┐\n│~~~~~~~~│\n└────────┘\n");
    CHECK_EQ(draw(box, Vector{8, 3}, active, Text{"A very long title"}), "┌A very┐\n│~~~~~~│\n└──────┘\n"); // clipped
    CHECK_EQ(draw(box, Vector{8, 3}, active, Text{"one\ntwo"}), "┌one───┐\n│~~~~~~│\n└──────┘\n"); // one line
    const BoxDecoration centered{BorderStyle{}, BorderStyle{}, Alignment::center};
    CHECK_EQ(draw(centered, Vector{10, 3}, active, Text{"Hi"}), "┌───Hi───┐\n│~~~~~~~~│\n└────────┘\n");
    CHECK_EQ(draw(centered, Vector{10, 3}, active, Text{"Hey"}), "┌──Hey───┐\n│~~~~~~~~│\n└────────┘\n");
    const BoxDecoration right{BorderStyle{}, BorderStyle{}, Alignment::right};
    CHECK_EQ(draw(right, Vector{10, 3}, active, Text{"Hi"}), "┌──────Hi┐\n│~~~~~~~~│\n└────────┘\n");
    CHECK_EQ(draw(right, Vector{10, 3}, active, Text{"long title text"}), "┌long tit┐\n│~~~~~~~~│\n└────────┘\n");
  }
  SUBCASE("Sizes") {
    CHECK_EQ(draw(box, Vector{2, 2}, active, Text{"T"}), "┌┐\n└┘\n"); // there is no room for a title
    CHECK_EQ(draw(box, Vector{3, 3}, active, Text{"T"}), "┌T┐\n│~│\n└─┘\n");
    CHECK_EQ(draw(box, Vector{1, 4}, active, Text{"T"}), "~\n~\n~\n~\n"); // there is no room for a frame
    CHECK_EQ(draw(box, Vector{5, 1}, active, Text{"T"}), "~~~~~\n");
  }
  SUBCASE("Colors") {
    const Char background{' ', RgbNone, RgbGray5};
    const auto drawn{[&](DecorationState state, const Text &title) {return box.draw(Vector{6, 3}, state, title, background, paint);}};
    SUBCASE("The roles of the state apply to the frame and the title") {
      const auto text{drawn(active, Text{"Hi"})};
      CHECK_EQ(text.data[0][0].attributes.fg, RgbBlue); // "frame" is the role of the active frame
      CHECK_EQ(text.data[2][5].attributes.fg, RgbBlue);
      CHECK_EQ(text.data[1][0].attributes.fg, RgbBlue);
      CHECK_EQ(text.data[0][1].attributes.fg, RgbYellow); // "title"
      CHECK_EQ(text.data[0][1].attributes.attr, bold);
      CHECK_EQ(text.data[0][3].attributes.fg, RgbBlue); // the rest of the top line is frame
      CHECK_EQ(text.data[0][3].attributes.attr, 0);
    }
    SUBCASE("The background shines through where a role does not have one") {
      const auto text{drawn(active, Text{"Hi"})};
      CHECK_EQ(text.data[0][0].attributes.bg, RgbGray5);
      CHECK_EQ(text.data[0][1].attributes.bg, RgbGray5);
      CHECK_EQ(text.data[1][0].attributes.bg, RgbGray5);
    }
    SUBCASE("Inside is left alone") {
      const auto text{drawn(active, Text{"Hi"})};
      CHECK_EQ(text.data[1][2], Untouched);
      CHECK_EQ(text.data[1][2].rune, NoneRune);
    }
    SUBCASE("The roles of the other state replace them, they do not add up") {
      const auto text{drawn(inactive, Text{"Hi"})};
      CHECK_EQ(text.data[0][0].attributes.fg, RgbGray5); // "frame.inactive"
      CHECK_EQ(text.data[0][1].attributes.fg, RgbGray5); // "title.inactive"
      CHECK_EQ(text.data[0][1].attributes.attr, underline); // and not bold anymore
    }
    SUBCASE("What the title says for itself wins") {
      const auto text{drawn(active, Text{"Hi", RgbRed})};
      CHECK_EQ(text.data[0][1].attributes.fg, RgbRed);
      CHECK_EQ(text.data[0][1].attributes.attr, bold); // the role is still there
      CHECK_EQ(text.data[0][1].rune, 'H');
    }
    SUBCASE("A style without roles changes nothing") {
      const auto text{box.draw(Vector{6, 3}, active, Text{"Hi"}, background, defaultColorStyle())};
      CHECK_EQ(text.data[0][0].attributes.fg, RgbNone);
      CHECK_EQ(text.data[0][0].attributes.bg, RgbGray5);
      CHECK_EQ(text.data[0][1].attributes.attr, 0);
    }
  }
}

TEST_CASE("UI style") {
  SUBCASE("The default style is the look the widgets always had") {
    const auto &style{defaultUiStyle()};
    CHECK_EQ(&style, &defaultUiStyle());
    CHECK_EQ(style.button[static_cast<std::size_t>(WidgetState::normal)].left, "[ ");
    CHECK_EQ(style.button[static_cast<std::size_t>(WidgetState::normal)].right, " ]");
    for (const auto state: {WidgetState::focus, WidgetState::active}) { // angle brackets show the focus
      CHECK_EQ(style.button[static_cast<std::size_t>(state)].left, "< ");
      CHECK_EQ(style.button[static_cast<std::size_t>(state)].right, " >");
    }
    for (const auto &brackets: style.dropDown) {
      CHECK_EQ(brackets.left, "[ ");
      CHECK_EQ(brackets.right, " ▼ ]");
    }
    CHECK_EQ(style.popup.left, " ");
    CHECK_EQ(style.popup.right, " ");
    CHECK_EQ(style.decoration, nullptr); // the default decoration
  }
  SUBCASE("A style is a constant value") {
    constexpr UiStyle style{
      .button = {Brackets{"(", ")"}, Brackets{"<", ">"}, Brackets{"{", "}"}},
    };
    static_assert(style.button[static_cast<std::size_t>(WidgetState::focus)].left == "<");
    CHECK_EQ(style.button[static_cast<std::size_t>(WidgetState::active)].right, "}");
    CHECK_EQ(style.dropDown[0].right, " ▼ ]"); // what is not said is the default
    UiStyle copy{style};
    copy.popup = Brackets{"|", "|"};
    CHECK_EQ(copy.popup.left, "|");
    CHECK_EQ(style.popup.left, " ");
  }
  SUBCASE("The default colors do not say anything, except what is needed to see anything") {
    const auto &colors{defaultColorStyle()};
    for (const auto *role: {"label", "button", "input", "dropdown", "popup"}) {
      INFO("Role: ", role);
      CHECK_NE(colors.find(role), nullptr);
      CHECK_EQ(colors[role], CharAttributes{});
    }
    CHECK_EQ(colors["popup.highlight"].attr, reverse);
    CHECK_EQ(colors["popup.highlight"].fg, RgbNone);
    CHECK_EQ(colors["popup.highlight"].bg, RgbNone);
    // the widget that gets the keyboard input is tinted; a pressed button and an open drop down have it
    for (const auto *role: {"input.focus", "button.focus", "button.active", "dropdown.focus", "dropdown.active"}) {
      INFO("Role: ", role);
      CHECK_EQ(colors[role].fg, RgbNone);
      CHECK_EQ(colors[role].bg, RgbGray5);
      CHECK_EQ(colors[role].attr, 0);
      CHECK_EQ(colors[role].mix, AttributeMode::mix); // into the background that is there
    }
    CHECK_EQ(colors["button.pressed"], CharAttributes{}); // a role nobody knows: nothing
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(readability-magic-numbers)
