#include "ui/action.hh"
#include "ui/button.hh"
#include "ui/color_style.hh"
#include "ui/decoration.hh"
#include "ui/frame.hh"
#include "ui/drop_down.hh"
#include "ui/horizontal_box.hh"
#include "ui/label.hh"
#include "ui/line_edit.hh"
#include "ui/text_area.hh"
#include "ui/vertical_box.hh"
#include "ui/widget.hh"
#include <ui/ui.hh>
#include <ui/window.hh>

#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <term/term.hh>
#include <term/text.hh>
#include <term/window.hh>

#include <cstdio>
#include <doctest/doctest.h>
#include <fstream>
#include <iterator>
#include <format>
#include <string>

using
  jwezel::AttributeMode::mix,
  jwezel::Char,
  jwezel::clear,
  jwezel::Line,
  jwezel::Rectangle,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin,
  jwezel::Window,
  jwezel::operator""_C;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTBEGIN(readability-function-cognitive-complexity)
// NOLINTBEGIN(hicpp-no-assembler)
// NOLINTBEGIN(hicpp-avoid-c-arrays)
// NOLINTBEGIN(hicpp-no-array-decay)
// NOLINTBEGIN(modernize-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(misc-use-anonymous-namespace)

namespace {

using jwezel::ui::ColorRoles, jwezel::ui::ColorStyle, jwezel::ui::DecorationState;

// A decoration without any frame
struct NoFrame: jwezel::ui::Decoration {
  [[nodiscard]] auto inset() const -> jwezel::Dim override {return 0;}

  [[nodiscard]] auto draw(
    const Vector &size, DecorationState, const Text &, const Char &, const ColorStyle &
  ) const -> Text override {
    return Text{jwezel::ui::Untouched, size};
  }
};

// A decoration that remembers the states it was asked to draw
struct Recording: jwezel::ui::BoxDecoration {
  [[nodiscard]] auto draw(
    const Vector &size, DecorationState state, const Text &title, const Char &background, const ColorStyle &colors
  ) const -> Text override {
    states.push_back(state);
    return BoxDecoration::draw(size, state, title, background, colors);
  }

  mutable std::vector<DecorationState> states;
};

constexpr ColorRoles<4> paintRoles{
  {"frame", jwezel::CharAttributes{jwezel::RgbBlue}},
  {"frame.inactive", jwezel::CharAttributes{jwezel::RgbGray5}},
  {"title", jwezel::CharAttributes{jwezel::RgbYellow, jwezel::RgbNone, jwezel::bold}},
  {"title.inactive", jwezel::CharAttributes{jwezel::RgbGray5, jwezel::RgbNone, jwezel::underline}},
};
constexpr ColorStyle paint{paintRoles};

// Colors for every widget; the attributes are there to see that they are taken away again
constexpr ColorRoles<10> widgetRoles{
  {"label", jwezel::CharAttributes{jwezel::RgbYellow}},
  {"button", jwezel::CharAttributes{jwezel::RgbBlue}},
  {"button.focus", jwezel::CharAttributes{jwezel::RgbGreen}},
  {"button.active", jwezel::CharAttributes{jwezel::RgbNone, jwezel::RgbRed, jwezel::reverse}},
  {"input", jwezel::CharAttributes{jwezel::RgbNone, jwezel::RgbGray2}},
  {"input.focus", jwezel::CharAttributes{jwezel::RgbNone, jwezel::RgbGray5}},
  {"dropdown", jwezel::CharAttributes{jwezel::RgbCyan}},
  {"dropdown.active", jwezel::CharAttributes{jwezel::RgbMagenta}},
  {"popup", jwezel::CharAttributes{jwezel::RgbNone, jwezel::RgbGray3}},
  {"popup.highlight", jwezel::CharAttributes{jwezel::RgbWhite, jwezel::RgbBlue5, jwezel::bold}},
};
constexpr ColorStyle widgetColors{widgetRoles};

using jwezel::ui::Brackets;

// Other glyphs for the widgets
constexpr jwezel::ui::UiStyle roundStyle{
  .button = {Brackets{"( ", " )"}, Brackets{"< ", " >"}, Brackets{"{ ", " }"}},
  .dropDown = {Brackets{"( ", " v )"}, Brackets{"( ", " v )"}, Brackets{"( ", " ^ )"}},
  .popup = Brackets{"|", "|"},
};

// Popup glyphs that do not add up to the width of the default ones
constexpr jwezel::ui::UiStyle wideStyle{
  .popup = Brackets{"<<", ">>>"},
};

// Colors without any role: for tests of what is where, not of how it looks
constexpr ColorRoles<1> noRoles{{"nothing", jwezel::CharAttributes{}}};
constexpr ColorStyle plain{noRoles};

// What the frame of a window looks like on the display: the default decoration, solid or (inactive) dashed
auto framed(const Vector &size, const Text &title, bool inactive = false) -> Text {
  auto result{Text{' '_C, size}};
  result.box(jwezel::Box{jwezel::RectangleMax, 1, inactive? jwezel::u1{1}: jwezel::u1{0}});
  result.patch(title, Vector{1, 0});
  return result;
}

} // namespace

TEST_CASE("UI Window") {
  char tempFilename[TMP_MAX];
  char outputFilename[TMP_MAX];
  (void)tmpnam(tempFilename); //NOLINT(concurrency-mt-unsafe)
  (void)tmpnam(outputFilename); //NOLINT(concurrency-mt-unsafe)
  auto *output{fopen(outputFilename, "ae")};
  auto *inputWrite{fopen(tempFilename, "we")};
  auto *inputRead{fopen(tempFilename, "re")};
  MESSAGE(std::string(tempFilename));
  MESSAGE(std::string(outputFilename));
  REQUIRE_NE(fputs("\x1b[1;1R", inputWrite), EOF);
  REQUIRE_NE(fputs("\x1b[1;1R", inputWrite), EOF);
  REQUIRE_NE(fputs("\x1b[30;40R", inputWrite), EOF);
  REQUIRE_FALSE(fflush(inputWrite));
  CAPTURE(ftell(inputWrite));
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, output->_fileno, inputRead->_fileno);
  jwezel::ui::Ui ui_{term};

  SUBCASE("Window") {
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 20, 12}, Text{"Title"}, nullptr, &plain)};
    auto test{Text{' '_C, Vector{20, 12}}};
    test.box();
    test.patch(Text{"Title"}, Vector{1, 0});
    CHECK_EQ(w1.text(), test);
    CHECK_EQ(term.display().text(), test);
    SUBCASE("Widget") {
      auto w2{jwezel::ui::Widget(Rectangle{1, 1, 8, 2}, ' '_C, w1)};
      w2.write(Vector{0, 0}, Text{"xxxxxxxxxxxxx"});
      auto test2{test};
      test2.patch(Text{"xxxxxxx"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Move widget") {
        w2.move(Rectangle{2, 1, 9, 2});
        auto test3{test};
        test3.patch(Text{"xxxxxxx"}, Vector{2, 1});
        CHECK_EQ(term.display().text(), test3);
      }
      SUBCASE("Mouse input") {
        bool called{false};
        REQUIRE_NE(fputs("\x1b[<0;3;2M", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        CAPTURE(ftell(inputWrite));
        w2.onMouseButton(
          [&called, &w2](const jwezel::ui::Element &el, const jwezel::Event &ev) {
            if (&el == &w2 and ev.type() == jwezel::MouseButtonEvent::type_) {
              called = true;
            }
            return true;
          }
        );
        CHECK_EQ(ftell(inputRead), 20);
        term.runEvent();
        CHECK_EQ(ftell(inputRead), 29);
        CHECK(called);
      }
      SUBCASE("Keyboard input") {
        bool called{false};
        REQUIRE_NE(fputs("x", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        CAPTURE(ftell(inputWrite));
        w2.onKey(
          [&called, &w2](const jwezel::ui::Element &el, const jwezel::Event &ev) {
            if (&el == &w2 and ev.type() == jwezel::KeyEvent::type_) {
              called = true;
            }
            return true;
          }
        );
        CHECK_EQ(ftell(inputRead), 20);
        term.runEvent();
        CHECK_EQ(ftell(inputRead), 21);
        CHECK(called);
      }
    }
    SUBCASE("Vertical box") {
      auto box{jwezel::ui::VerticalBox(w1, Rectangle{1, 1, 19, 11})};
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{1, 1, 19, 4}));
      CHECK_EQ(w3.area(), (Rectangle{1, 4, 19, 8}));
      CHECK_EQ(w4.area(), (Rectangle{1, 8, 19, 11}));
    }
    SUBCASE("Vertical spacing") {
      auto box{jwezel::ui::VerticalBox(w1, Rectangle{1, 1, 19, 11}, jwezel::ui::BoxLayout{.spacing = 1})};
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{1, 1, 19, 4}));
      CHECK_EQ(w3.area(), (Rectangle{1, 5, 19, 7}));
      CHECK_EQ(w4.area(), (Rectangle{1, 8, 19, 11}));
    }
    SUBCASE("Vertical padding") {
      auto box{
        jwezel::ui::VerticalBox(w1, Rectangle{1, 1, 19, 11}, jwezel::ui::BoxLayout{.spacing = 1, .padding = 1})
      };
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{2, 2, 18, 4}));
      CHECK_EQ(w3.area(), (Rectangle{2, 5, 18, 7}));
      CHECK_EQ(w4.area(), (Rectangle{2, 8, 18, 10}));
    }
    SUBCASE("Vertical weights") {
      auto box{jwezel::ui::VerticalBox(w1, Rectangle{1, 1, 19, 11}, jwezel::ui::BoxLayout{.weights = {1, 2, 3}})};
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{1, 1, 19, 3}));
      CHECK_EQ(w3.area(), (Rectangle{1, 3, 19, 6}));
      CHECK_EQ(w4.area(), (Rectangle{1, 6, 19, 11}));
    }
    SUBCASE("Horizontal box") {
      auto box{jwezel::ui::HorizontalBox(w1, Rectangle{1, 1, 19, 11})};
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{1, 1, 7, 11}));
      CHECK_EQ(w3.area(), (Rectangle{7, 1, 13, 11}));
      CHECK_EQ(w4.area(), (Rectangle{13, 1, 19, 11}));
    }
    SUBCASE("Horizontal spacing") {
      auto box{jwezel::ui::HorizontalBox(w1, Rectangle{1, 1, 19, 11}, jwezel::ui::BoxLayout{.spacing = 1})};
      auto w2{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w3{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      auto w4{jwezel::ui::Widget(jwezel::RectangleDefault, ' '_C, box)};
      CHECK_EQ(w2.area(), (Rectangle{1, 1, 6, 11}));
      CHECK_EQ(w3.area(), (Rectangle{7, 1, 13, 11}));
      CHECK_EQ(w4.area(), (Rectangle{14, 1, 19, 11}));
    }
    SUBCASE("Label") {
      auto label{jwezel::ui::Label(w1, Rectangle{1, 1, 19, 3}, Text{"Hello"})};
      auto test2{test};
      test2.patch(Text{"Hello"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Move label") {
        label.move(Rectangle{2, 2, 19, 4});
        auto test3{test};
        test3.patch(Text{"Hello"}, Vector{2, 2});
        CHECK_EQ(term.display().text(), test3);
      }
      SUBCASE("Change the text") {
        label.content(Text{"Hi\nthere"});
        CHECK_EQ(label.content(), Text{"Hi\nthere"});
        auto test3{test};
        test3.patch(Text{"Hi\nthere"}, Vector{1, 1});
        CHECK_EQ(term.display().text(), test3); // nothing is left of "Hello"
        label.content(Text{"x"});
        auto test4{test};
        test4.patch(Text{"x"}, Vector{1, 1});
        CHECK_EQ(term.display().text(), test4); // nor of the second line
      }
    }
    SUBCASE("Label in box") {
      auto box{jwezel::ui::VerticalBox(w1, Rectangle{1, 1, 19, 9})};
      auto label{jwezel::ui::Label(box, Text{"Hello"})};
      auto label2{jwezel::ui::Label(box, Text{"World"})};
      auto test2{test};
      test2.patch(Text{"Hello"}, Vector{1, 1});
      test2.patch(Text{"World"}, Vector{1, 5});
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Clip label") {
        auto label3{jwezel::ui::Label(w1, Rectangle{1, 9, 19, 11}, Text{"Hello World and much more"})};
        auto test3{test2};
        test3.patch(Text{"Hello World and mu"}, Vector{1, 9});
        CHECK_EQ(term.display().text(), test3);
      }
    }
    SUBCASE("Button") {
      auto button{jwezel::ui::Button(w1, Rectangle{1, 1, 8, 2}, Text{"OK"})};
      auto test2{test};
      test2.patch(Text{"< OK >"}, Vector{1, 1}); // it has the focus
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Change the text") {
        button.content(Text{"Cancel"});
        auto test3{test};
        test3.patch(Text{"< Cance"}, Vector{1, 1}); // the button has the focus; its area is 7 wide, the rest is clipped
        CHECK_EQ(term.display().text(), test3);
        button.content(Text{"No"});
        auto test4{test};
        test4.patch(Text{"< No >"}, Vector{1, 1}); // and nothing is left of the longer one
        CHECK_EQ(term.display().text(), test4);
      }
      SUBCASE("Press with space") {
        bool called{false};
        REQUIRE_NE(fputs(" ", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        CAPTURE(ftell(inputWrite));
        button.onPress(
          [&called](const jwezel::ui::Element &, const jwezel::Event &) {
            called = true;
            return true;
          }
        );
        CHECK_EQ(ftell(inputRead), 20);
        term.runEvent();
        CHECK_EQ(ftell(inputRead), 21);
        CHECK(called);
      }
      SUBCASE("Click with mouse") {
        bool called{false};
        REQUIRE_NE(fputs("\x1b[<0;3;2M", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        CAPTURE(ftell(inputWrite));
        button.onPress(
          [&called](const jwezel::ui::Element &, const jwezel::Event &) {
            called = true;
            return true;
          }
        );
        CHECK_EQ(ftell(inputRead), 20);
        term.runEvent();
        CHECK_EQ(ftell(inputRead), 29);
        CHECK_FALSE(called); // a press alone is not a click
        REQUIRE_NE(fputs("\x1b[<0;3;2m", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        CHECK_EQ(ftell(inputRead), 38);
        CHECK(called);
      }
      SUBCASE("Click calls off by moving out") {
        int called{0};
        button.onPress(
          [&called](const jwezel::ui::Element &, const jwezel::Event &) {
            ++called;
            return true;
          }
        );
        auto send{
          [&](const char *input) {
            REQUIRE_NE(fputs(input, inputWrite), EOF);
            REQUIRE_FALSE(fflush(inputWrite));
            term.runEvent();
          }
        };
        send("\x1b[<0;3;2M"); // press inside
        send("\x1b[<32;11;2M"); // drag out (motion with the button down)
        send("\x1b[<0;11;2m"); // release outside: the release still reaches the button
        CHECK_EQ(called, 0);
        send("\x1b[<0;3;2m"); // a stray release inside does not click either
        CHECK_EQ(called, 0);
        send("\x1b[<0;11;2M"); // press outside
        send("\x1b[<32;3;2M"); // drag in
        send("\x1b[<0;3;2m"); // release inside
        CHECK_EQ(called, 0);
        send("\x1b[<0;3;2M");
        send("\x1b[<32;4;2M"); // moving inside is fine
        send("\x1b[<0;4;2m");
        CHECK_EQ(called, 1);
        send("\x1b[<64;3;2M"); // the wheel is not a click
        CHECK_EQ(called, 1);
        send("\x1b[<1;3;2M"); // neither is another button
        send("\x1b[<1;3;2m");
        CHECK_EQ(called, 1);
      }
      SUBCASE("Mouse outside") {
        bool called{false};
        REQUIRE_NE(fputs("\x1b[<0;11;2M", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        CAPTURE(ftell(inputWrite));
        button.onPress(
          [&called](const jwezel::ui::Element &, const jwezel::Event &) {
            called = true;
            return true;
          }
        );
        term.runEvent();
        CHECK_FALSE(called);
      }
    }
    SUBCASE("Text input") {
      auto input{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 19, 2}, Text{"Hello"})};
      auto test2{test};
      test2.patch(Text{"Hello"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      CHECK_EQ(term.display().caret(), Vector{6, 1});
      SUBCASE("Type") {
        REQUIRE_NE(fputs("X", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        auto test3{test};
        test3.patch(Text{"HelloX"}, Vector{1, 1});
        CHECK_EQ(term.display().text(), test3);
        CHECK_EQ(input.content().data[0][5].rune, 'X');
        CHECK_EQ(term.display().caret(), Vector{7, 1});
      }
      SUBCASE("Arrow left and type") {
        REQUIRE_NE(fputs("\x1b[D\x1b[D\x1b[DX", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        term.runEvent();
        term.runEvent();
        term.runEvent();
        auto test3{test};
        test3.patch(Text{"HeXllo"}, Vector{1, 1});
        CHECK_EQ(term.display().text(), test3);
        CHECK_EQ(input.content().data[0][2].rune, 'X');
        CHECK_EQ(term.display().caret(), Vector{4, 1});
      }
      SUBCASE("Backspace") {
        REQUIRE_NE(fputs("\x7f", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        auto test3{test};
        test3.patch(Text{"Hell"}, Vector{1, 1});
        CHECK_EQ(term.display().text(), test3);
      }
      SUBCASE("Enter accepts") {
        bool called{false};
        REQUIRE_NE(fputs("\x0d", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        input.onAccept(
          [&called](const jwezel::ui::Element &, const jwezel::Event &) {
            called = true;
            return true;
          }
        );
        term.runEvent();
        CHECK(called);
        CHECK_EQ(input.content().data.size(), 1);
      }
    }
    SUBCASE("Scrolling input") {
      auto input{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 7, 2}, Text{"HelloWorld"})};
      auto test2{test};
      test2.patch(Text{"World"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      REQUIRE_NE(fputs("X", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      auto test3{test};
      test3.patch(Text{"orldX"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test3);
    }
    SUBCASE("Text area") {
      auto area{jwezel::ui::TextArea(w1, Rectangle{1, 1, 19, 4}, Text{"Hello"})};
      auto test2{test};
      test2.patch(Text{"Hello"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Enter inserts newline") {
        REQUIRE_NE(fputs("\x0dWorld", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        term.runEvent();
        term.runEvent();
        term.runEvent();
        term.runEvent();
        term.runEvent();
        auto test3{test};
        test3.patch(Text{"Hello"}, Vector{1, 1});
        test3.patch(Text{"World"}, Vector{1, 2});
        CHECK_EQ(term.display().text(), test3);
        CHECK_EQ(area.content().data.size(), 2);
      }
      SUBCASE("Backspace joins lines") {
        REQUIRE_NE(fputs("\x0d\x7f", inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
        term.runEvent();
        CHECK_EQ(area.content().data.size(), 1);
        CHECK_EQ(area.content().data[0].size(), 5);
      }
    }
    SUBCASE("Click to focus") {
      auto input1{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 19, 2}, Text{"One"})};
      auto input2{jwezel::ui::LineEdit(w1, Rectangle{1, 3, 19, 4}, Text{"Two"})};
      REQUIRE_NE(fputs("\x1b[<0;3;4M", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      REQUIRE_NE(fputs("X", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      auto test2{test};
      test2.patch(Text{"One"}, Vector{1, 1});
      test2.patch(Text{"TwoX"}, Vector{1, 3});
      CHECK_EQ(term.display().text(), test2);
      CHECK_EQ(input2.content().data[0][3].rune, 'X');
      CHECK_EQ(input1.content().data[0].size(), 3);
      CHECK_EQ(term.display().caret(), Vector{5, 3});
    }
    CHECK_EQ(term.display().text(), test);
  }

  SUBCASE("Offset window") {
    const Rectangle placement{4, 3, 24, 15};
    auto w1{jwezel::ui::Window(ui_, ' '_C, placement, Text{"Title"}, nullptr, &plain)};
    auto test{Text{' '_C, Vector{20, 12}}};
    test.box();
    test.patch(Text{"Title"}, Vector{1, 0});
    auto input1{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 19, 2}, Text{"One"})};
    auto input2{jwezel::ui::LineEdit(w1, Rectangle{1, 3, 19, 4}, Text{"Two"})};
    auto button{jwezel::ui::Button(w1, Rectangle{1, 5, 19, 6}, Text{"Ok"})};
    test.patch(Text{"One"}, Vector{1, 1});
    test.patch(Text{"Two"}, Vector{1, 3});
    test.patch(Text{"[ Ok ]"}, Vector{1, 5});
    CHECK_EQ(input1.bounds(), Rectangle{1, 1, 19, 2});
    CHECK_EQ(term.display().text()[placement], test);
    CHECK_EQ(term.display().caret(), Vector{8, 4}); // display position, end of "One"
    SUBCASE("Click to focus") {
      REQUIRE_NE(fputs("\x1b[<0;7;7M", inputWrite), EOF); // window-relative (2, 3), display (6, 6)
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      REQUIRE_NE(fputs("X", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      test.patch(Text{"TwoX"}, Vector{1, 3});
      CHECK_EQ(term.display().text()[placement], test);
      CHECK_EQ(input1.content().data[0].size(), 3);
      CHECK_EQ(input2.content().data[0].size(), 4);
      CHECK_EQ(term.display().caret(), Vector{9, 6});
      std::ifstream written{outputFilename};
      const std::string output_{std::istreambuf_iterator<char>{written}, {}};
      // Hide, paint X at display (8, 6), show again: the cursor ends up right behind X, at the caret (9, 6)
      CHECK(output_.ends_with("\x1b[7;10H\x1b[?25l\x1b[7;9HX\x1b[?25h"));
    }
    SUBCASE("Click button") {
      bool pressed{false};
      button.onPress(
        [&pressed](const jwezel::ui::Element &, const jwezel::Event &) {
          pressed = true;
          return true;
        }
      );
      REQUIRE_NE(fputs("\x1b[<0;7;9M", inputWrite), EOF); // window-relative (2, 5), display (6, 8)
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      CHECK_FALSE(pressed);
      CHECK_FALSE(term.display().caret()); // a button has no caret
      REQUIRE_NE(fputs("\x1b[<0;7;9m", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      CHECK(pressed);
    }
    SUBCASE("Click on focused widget keeps focus") {
      REQUIRE_NE(fputs("\x1b[<0;7;5M", inputWrite), EOF); // window-relative (2, 1) is input1
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      REQUIRE_NE(fputs("X", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      CHECK_EQ(input1.content().data[0].size(), 4);
    }
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    SUBCASE("Click on frame is ignored") {
      send("\x1b[<0;5;4M"); // display (4, 3): top left corner of the window frame
      send("X");
      CHECK_EQ(input1.content().data[0].size(), 4); // focus unchanged
      CHECK_EQ(input2.content().data[0].size(), 3);
    }
    SUBCASE("Covered widget") {
      // Display areas: input1 {5, 4, 23, 5}, input2 {5, 6, 23, 7}
      SUBCASE("Completely covered gets no keyboard input") {
        {
          Window cover{&term, Rectangle{5, 4, 23, 5}, '#'_C};
          term.focus(&w1);
          CHECK_FALSE(term.display().caret()); // the caret is hidden as well
          send("X");
          CHECK_EQ(input1.content().data[0].size(), 3);
        }
        CHECK_EQ(term.display().caret(), Vector{8, 4}); // uncovered: caret is back
        send("X");
        CHECK_EQ(input1.content().data[0].size(), 4);
        CHECK_EQ(term.display().caret(), Vector{9, 4});
      }
      SUBCASE("Partly covered still gets keyboard input") {
        Window cover{&term, Rectangle{15, 4, 23, 5}, '#'_C};
        term.focus(&w1);
        CHECK_EQ(term.display().caret(), Vector{8, 4}); // caret cell itself is visible
        send("X");
        CHECK_EQ(input1.content().data[0].size(), 4);
        CHECK_EQ(term.display().caret(), Vector{9, 4});
      }
      SUBCASE("Caret cell covered") {
        Window cover{&term, Rectangle{8, 4, 23, 5}, '#'_C};
        term.focus(&w1);
        CHECK_FALSE(term.display().caret());
        send("X"); // the widget is still partly visible
        CHECK_EQ(input1.content().data[0].size(), 4);
        CHECK_FALSE(term.display().caret());
      }
      SUBCASE("Click on covered widget is ignored") {
        Window cover{&term, Rectangle{5, 6, 23, 7}, '#'_C};
        term.focus(&w1);
        send("\x1b[<0;7;7M"); // display (6, 6): input2, covered
        send("X");
        CHECK_EQ(input1.content().data[0].size(), 4); // focus did not move
        CHECK_EQ(input2.content().data[0].size(), 3);
      }
      SUBCASE("Click on uncovered part of a widget still focuses it") {
        Window cover{&term, Rectangle{15, 6, 23, 7}, '#'_C};
        term.focus(&w1);
        send("\x1b[<0;7;7M"); // display (6, 6): input2, visible
        send("X");
        CHECK_EQ(input2.content().data[0].size(), 4);
        CHECK_EQ(input1.content().data[0].size(), 3);
      }
    }
    SUBCASE("Two windows") {
      auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{26, 3, 40, 10}, Text{"Other"})};
      auto other{jwezel::ui::LineEdit(w2, Rectangle{1, 1, 13, 2}, Text{"AB"})};
      CHECK_EQ(term.focus(), &w2); // the newest window has the focus
      CHECK_EQ(term.display().caret(), Vector{29, 4});
      SUBCASE("Click on widget of another window focuses it") {
        send("\x1b[<0;7;7M"); // display (6, 6): input2 of w1
        CHECK_EQ(term.focus(), &w1);
        CHECK_EQ(w1.focus(), &input2);
        CHECK_EQ(term.display().caret(), Vector{8, 6}); // w2's caret is gone, w1's is shown
        send("X");
        CHECK_EQ(input2.content().data[0].size(), 4);
        CHECK_EQ(other.content().data[0].size(), 2);
        CHECK_EQ(term.display().caret(), Vector{9, 6});
      }
      SUBCASE("Click on frame of another window focuses it") {
        send("\x1b[<0;5;4M"); // display (4, 3): frame of w1
        CHECK_EQ(term.focus(), &w1);
        CHECK_EQ(w1.focus(), &input1); // the focused widget of w1 is unchanged
        CHECK_EQ(term.display().caret(), Vector{8, 4});
        send("X");
        CHECK_EQ(input1.content().data[0].size(), 4);
        CHECK_EQ(other.content().data[0].size(), 2);
      }
      SUBCASE("Click back") {
        send("\x1b[<0;7;7M");
        CHECK_EQ(term.focus(), &w1);
        send("\x1b[<0;30;5M"); // display (29, 4): other
        CHECK_EQ(term.focus(), &w2);
        CHECK_EQ(term.display().caret(), Vector{29, 4});
        send("X");
        CHECK_EQ(other.content().data[0].size(), 3);
        CHECK_EQ(input2.content().data[0].size(), 3);
      }
      SUBCASE("Click on desktop focuses it") {
        send("\x1b[<0;31;13M"); // display (30, 12): bare desktop
        CHECK_EQ(term.focus(), &term.desktop());
        CHECK_FALSE(term.display().caret()); // nothing on the desktop has a caret
        send("X"); // the desktop has no widget to type into
        CHECK_EQ(other.content().data[0].size(), 2);
        CHECK_EQ(input1.content().data[0].size(), 3);
        send("\x1b[<0;30;5M"); // and clicking another window takes the focus away from it
        CHECK_EQ(term.focus(), &w2);
        CHECK_EQ(term.display().caret(), Vector{29, 4});
      }
      SUBCASE("Mouse move does not change focus") {
        send("\x1b[<35;7;7M"); // button-less motion over input2 of w1
        CHECK_EQ(term.focus(), &w2);
      }
      SUBCASE("Press, drag and release stay with the window of the press") {
        int w1Buttons{0}, w1Moves{0}, w2Buttons{0}, w2Moves{0};
        input2.onMouseButton(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++w1Buttons;
            return false;
          }
        );
        input2.onMouseMove(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++w1Moves;
            return false;
          }
        );
        other.onMouseButton(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++w2Buttons;
            return false;
          }
        );
        other.onMouseMove(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++w2Moves;
            return false;
          }
        );
        send("\x1b[<0;7;7M"); // press on input2 of w1
        CHECK_EQ(term.focus(), &w1);
        send("\x1b[<32;31;5M"); // drag over the input of w2
        send("\x1b[<0;31;5m"); // release over it
        CHECK_EQ(w1Buttons, 2); // press and release
        CHECK_EQ(w1Moves, 1);
        CHECK_EQ(w2Buttons, 0);
        CHECK_EQ(w2Moves, 0);
        CHECK_EQ(term.focus(), &w1); // releasing over w2 did not focus it
        send("\x1b[<35;31;5M"); // motion without a button goes to the window under the mouse again
        CHECK_EQ(w2Moves, 1);
        CHECK_EQ(w1Moves, 1);
        send("\x1b[<0;31;5M"); // a new press over w2 is for w2
        CHECK_EQ(term.focus(), &w2);
        CHECK_EQ(w2Buttons, 1);
        send("\x1b[<0;31;5m");
        CHECK_EQ(w2Buttons, 2);
        CHECK_EQ(w1Buttons, 2);
      }
      SUBCASE("Press on frame is not for the widgets it is dragged over") {
        int buttons{0}, moves{0};
        input2.onMouseButton(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++buttons;
            return false;
          }
        );
        input2.onMouseMove(
          [&](const jwezel::ui::Element &, const jwezel::Event &) {
            ++moves;
            return false;
          }
        );
        send("\x1b[<0;5;4M"); // press on the frame of w1
        send("\x1b[<32;7;7M"); // drag over input2
        send("\x1b[<0;7;7m"); // release over it
        CHECK_EQ(buttons, 0);
        CHECK_EQ(moves, 0);
        CHECK_EQ(term.focus(), &w1);
        CHECK_EQ(w1.focus(), &input1);
      }
      SUBCASE("Lost release") {
        send("\x1b[<0;7;7M"); // press on input2 of w1, the release never arrives
        send("\x1b[<0;30;5M"); // the next press starts over, at w2
        CHECK_EQ(term.focus(), &w2);
        send("\x1b[<0;30;5m");
      }
    }
    SUBCASE("Destroyed focused window hands focus to the desktop") {
      {
        Window extra{&term, Rectangle{26, 3, 40, 10}, '#'_C};
        CHECK_EQ(term.focus(), &extra);
      }
      CHECK_EQ(term.focus(), &term.desktop());
      send("X"); // goes nowhere, but must not crash
      CHECK_EQ(input1.content().data[0].size(), 3);
      send("\x1b[<0;7;7M"); // click on input2 of w1 gets the focus back
      CHECK_EQ(term.focus(), &w1);
      send("X");
      CHECK_EQ(input2.content().data[0].size(), 4);
    }
    SUBCASE("Move window") {
      const Rectangle moved{10, 6, 30, 18};
      w1.move(moved);
      CHECK_EQ(input1.bounds(), Rectangle{1, 1, 19, 2});
      CHECK_EQ(term.display().text()[moved], test);
      REQUIRE_NE(fputs("\x1b[<0;13;10M", inputWrite), EOF); // window-relative (2, 3), display (12, 9)
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      REQUIRE_NE(fputs("X", inputWrite), EOF);
      REQUIRE_FALSE(fflush(inputWrite));
      term.runEvent();
      CHECK_EQ(input2.content().data[0].size(), 4);
      CHECK_EQ(term.display().caret(), Vector{15, 9});
    }
    SUBCASE("Move window carries caret") {
      w1.move(Rectangle{10, 6, 30, 18});
      CHECK_EQ(term.display().caret(), Vector{14, 7});
    }
  }

  SUBCASE("Desktop") {
    auto &desktop{ui_.desktop()};
    desktop.colors(&plain);
    auto input{jwezel::ui::LineEdit(desktop, Rectangle{2, 1, 12, 2}, Text{"Hi"})};
    auto button{jwezel::ui::Button(desktop, Rectangle{2, 3, 12, 4}, Text{"Go"})};
    int clicks{0};
    button.onPress(
      [&clicks](const jwezel::ui::Element &, const jwezel::Event &) {
        ++clicks;
        return true;
      }
    );
    auto send{
      [&](const char *input_) {
        REQUIRE_NE(fputs(input_, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    CHECK_EQ(term.focus(), &term.desktop());
    CHECK_EQ(desktop.focus(), &input); // the first widget starts focused, like in any window
    CHECK_EQ(term.display().caret(), Vector{4, 1});
    CHECK_EQ(term.display().text()[Rectangle{2, 1, 4, 2}], Text{"Hi"});
    CHECK_EQ(term.display().text()[Rectangle{2, 3, 8, 4}], Text{"[ Go ]"});
    SUBCASE("Type") {
      send("X");
      CHECK_EQ(input.content().data[0].size(), 3);
      CHECK_EQ(term.display().text()[Rectangle{2, 1, 5, 2}], Text{"HiX"});
      CHECK_EQ(term.display().caret(), Vector{5, 1});
    }
    SUBCASE("Click button") {
      send("\x1b[<0;4;4M"); // display (3, 3)
      CHECK_EQ(clicks, 0);
      CHECK_EQ(desktop.focus(), &button);
      CHECK_FALSE(term.display().caret());
      send("\x1b[<0;4;4m");
      CHECK_EQ(clicks, 1);
    }
    SUBCASE("Click input") {
      send("\x1b[<0;4;4M");
      send("\x1b[<0;4;4m");
      CHECK_EQ(desktop.focus(), &button);
      send("\x1b[<0;4;2M"); // display (3, 1)
      send("\x1b[<0;4;2m");
      CHECK_EQ(desktop.focus(), &input);
      CHECK_EQ(term.display().caret(), Vector{4, 1});
      send("X");
      CHECK_EQ(input.content().data[0].size(), 3);
    }
    SUBCASE("Box") {
      auto box{jwezel::ui::VerticalBox(desktop, Rectangle{0, 5, 10, 9})};
      auto top{jwezel::ui::LineEdit(box, Text{"A"})};
      auto bottom{jwezel::ui::LineEdit(box, Text{"B"})};
      CHECK_EQ(top.bounds(), Rectangle{0, 5, 10, 7});
      CHECK_EQ(bottom.bounds(), Rectangle{0, 7, 10, 9});
      CHECK_EQ(term.display().text()[Rectangle{0, 7, 1, 8}], Text{"B"});
      send("\x1b[<0;3;8M"); // display (2, 7): bottom
      CHECK_EQ(desktop.focus(), &bottom);
      send("X");
      CHECK_EQ(bottom.content().data[0].size(), 2);
      CHECK_EQ(top.content().data[0].size(), 1);
      CHECK_EQ(term.display().caret(), Vector{2, 7});
    }
    SUBCASE("Window over the desktop") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{6, 0, 20, 6}, Text{"Title"})};
      auto inner{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 13, 2}, Text{"In"})};
      CHECK_EQ(term.focus(), &w1);
      CHECK_EQ(term.display().caret(), Vector{9, 1});
      send("X");
      CHECK_EQ(inner.content().data[0].size(), 3);
      CHECK_EQ(input.content().data[0].size(), 2);
      send("\x1b[<0;4;2M"); // display (3, 1): the input on the desktop, not covered by the window
      send("\x1b[<0;4;2m");
      CHECK_EQ(term.focus(), &term.desktop());
      CHECK_EQ(term.display().caret(), Vector{4, 1});
      send("Y");
      CHECK_EQ(input.content().data[0].size(), 3);
      CHECK_EQ(inner.content().data[0].size(), 3);
    }
    SUBCASE("Widgets leave when destroyed") {
      {
        auto extra{jwezel::ui::LineEdit(desktop, Rectangle{2, 5, 12, 6}, Text{"Tmp"})};
        send("\x1b[<0;4;6M"); // display (3, 5)
        send("\x1b[<0;4;6m");
        CHECK_EQ(desktop.focus(), &extra);
        CHECK_EQ(term.display().caret(), Vector{5, 5});
        CHECK_EQ(desktop.children().size(), 3);
      }
      CHECK_EQ(desktop.focus(), nullptr);
      CHECK_EQ(desktop.children().size(), 2);
      CHECK_FALSE(term.display().caret());
      send("X"); // nowhere to go, must not crash
      send("\x1b[<0;4;6M"); // nor a click where the widget was
      send("\x1b[<0;4;6m");
      CHECK_EQ(input.content().data[0].size(), 2);
    }
  }

  SUBCASE("Desktop layout") {
    Window extra{&term, Rectangle{0, 0, 20, 12}, '#'_C}; // the display, and with it the desktop, is 20 x 12 now
    auto &desktop{ui_.desktop()};
    CHECK_EQ(desktop.bounds(), Rectangle{0, 0, 20, 12});
    auto box{jwezel::ui::VerticalBox(desktop, jwezel::RectangleDefault)};
    auto top{jwezel::ui::LineEdit(box, Text{"A"})};
    auto bottom{jwezel::ui::LineEdit(box, Text{"B"})};
    auto fixed{jwezel::ui::Label(desktop, Rectangle{2, 2, 6, 3}, Text{"ok"})};
    CHECK_EQ(box.bounds(), Rectangle{0, 0, 20, 12});
    CHECK_EQ(top.bounds(), Rectangle{0, 0, 20, 6});
    CHECK_EQ(bottom.bounds(), Rectangle{0, 6, 20, 12});
    CHECK_EQ(fixed.bounds(), Rectangle{2, 2, 6, 3});
    SUBCASE("The desktop grows") {
      Window bigger{&term, Rectangle{0, 0, 30, 15}, '#'_C};
      CHECK_EQ(desktop.bounds(), Rectangle{0, 0, 30, 15});
      CHECK_EQ(box.bounds(), Rectangle{0, 0, 30, 15}); // fills the desktop
      CHECK_EQ(top.bounds(), Rectangle{0, 0, 30, 8});
      CHECK_EQ(bottom.bounds(), Rectangle{0, 8, 30, 15});
      CHECK_EQ(fixed.bounds(), Rectangle{2, 2, 6, 3}); // has an area of its own
      CHECK_EQ(top.area(), Rectangle{0, 0, 30, 8});
    }
    SUBCASE("A box with an area of its own does not fill") {
      auto sized{jwezel::ui::HorizontalBox(desktop, Rectangle{1, 1, 11, 5})};
      Window bigger{&term, Rectangle{0, 0, 30, 15}, '#'_C};
      CHECK_EQ(sized.bounds(), Rectangle{1, 1, 11, 5});
    }
  }

  SUBCASE("Window resize") {
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{2, 1, 22, 13}, Text{"T"})};
    auto box{jwezel::ui::VerticalBox(w1, jwezel::RectangleDefault)};
    auto input{jwezel::ui::LineEdit(box, Text{"Hi"})};
    auto fixed{jwezel::ui::Label(w1, Rectangle{1, 1, 3, 2}, Text{"ok"})};
    auto framed{
      [](const Vector &size) {
        auto result{Text{' '_C, size}};
        result.box();
        result.patch(Text{"T"}, Vector{1, 0});
        return result;
      }
    };
    CHECK_EQ(w1.content(), Rectangle{1, 1, 19, 11}); // inside the frame
    CHECK_EQ(box.bounds(), Rectangle{1, 1, 19, 11});
    CHECK_EQ(input.bounds(), Rectangle{1, 1, 19, 11});
    CHECK_EQ(w1.text(), framed(Vector{20, 12}));
    SUBCASE("Grow") {
      w1.move(Rectangle{2, 1, 32, 16});
      CHECK_EQ(w1.content(), Rectangle{1, 1, 29, 14});
      CHECK_EQ(box.bounds(), Rectangle{1, 1, 29, 14});
      CHECK_EQ(input.bounds(), Rectangle{1, 1, 29, 14});
      CHECK_EQ(input.area(), Rectangle{3, 2, 31, 15}); // on the display
      CHECK_EQ(fixed.bounds(), Rectangle{1, 1, 3, 2});
      CHECK_EQ(w1.text(), framed(Vector{30, 15})); // the frame has been drawn again
      CHECK_EQ(term.display().caret(), Vector{5, 2}); // end of "Hi"
    }
    SUBCASE("Shrink") {
      w1.move(Rectangle{2, 1, 12, 8});
      CHECK_EQ(box.bounds(), Rectangle{1, 1, 9, 6});
      CHECK_EQ(input.bounds(), Rectangle{1, 1, 9, 6});
      CHECK_EQ(w1.text(), framed(Vector{10, 7}));
    }
    SUBCASE("Move only") {
      w1.move(Rectangle{6, 4, 26, 16});
      CHECK_EQ(box.bounds(), Rectangle{1, 1, 19, 11}); // relative to the window: nothing changed
      CHECK_EQ(input.area(), Rectangle{7, 5, 25, 15});
      CHECK_EQ(fixed.area(), Rectangle{7, 5, 9, 6});
      CHECK_EQ(term.display().caret(), Vector{9, 5});
    }
    SUBCASE("Moved by the terminal") {
      term.moveWindow(w1, Rectangle{6, 4, 36, 19}); // moved and resized, not through the window
      CHECK_EQ(box.bounds(), Rectangle{1, 1, 29, 14});
      CHECK_EQ(input.area(), Rectangle{7, 5, 35, 18});
      CHECK_EQ(fixed.area(), Rectangle{7, 5, 9, 6});
      CHECK_EQ(w1.text(), framed(Vector{30, 15}));
    }
    SUBCASE("Without room for content") {
      w1.move(Rectangle{2, 1, 4, 3}); // nothing left inside the frame
      CHECK_EQ(w1.content().width(), 0);
      CHECK_EQ(w1.text().size(), Vector{2, 2});
      w1.move(Rectangle{2, 1, 22, 13}); // and back
      CHECK_EQ(box.bounds(), Rectangle{1, 1, 19, 11});
      CHECK_EQ(input.bounds(), Rectangle{1, 1, 19, 11});
    }
  }

  SUBCASE("Drop down") {
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 20, 12}, Text{"Title"})};
    const std::vector<Text> fruit{Text{"Apple"}, Text{"Banana"}, Text{"Cherry"}};
    auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 1, 19, 2}, fruit, 1)};
    auto field{jwezel::ui::LineEdit(w1, Rectangle{1, 3, 19, 4}, Text{"x"})};
    auto field2{jwezel::ui::LineEdit(w1, Rectangle{1, 6, 19, 7}, Text{"y"})};
    int chosen{0};
    drop.onSelect(
      [&chosen](const jwezel::ui::Element &, const jwezel::Event &) {
        ++chosen;
        return true;
      }
    );
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    auto padded{[](const std::string &text) {return text + std::string(18 - text.size(), ' ');}};
    auto closed{[&](const std::string &item) {return "[ " + item + std::string(12 - item.size(), ' ') + " ▼ ]";}};
    auto row{
      [&](int line) { // the characters of a row; how it looks is checked separately
        auto result{term.display().text()[Rectangle{1, jwezel::toDim(line), 19, jwezel::toDim(line + 1)}]};
        for (auto &cell: result.data.at(0)) {
          cell.attributes = jwezel::CharAttributes{}; // the content, not how it looks: the highlight, the tint
        }
        return result;
      }
    };
    auto reversed{
      [&](int line) {
        return (term.display().text().data[static_cast<size_t>(line)][1].attributes.attr & jwezel::reverse) != 0;
      }
    };
    CHECK_EQ(drop.selected(), 1);
    CHECK_FALSE(drop.isOpen());
    CHECK_EQ(w1.focus(), &drop);
    CHECK_EQ(row(1), Text{closed("Banana")});
    CHECK_EQ(row(3), Text{padded("x")});
    CHECK_FALSE(term.display().caret());
    SUBCASE("Open") {
      send(" ");
      CHECK(drop.isOpen());
      CHECK_EQ(row(2), Text{padded(" Apple")});
      CHECK_EQ(row(3), Text{padded(" Banana")}); // in front of the field
      CHECK_EQ(row(4), Text{padded(" Cherry")});
      CHECK_EQ(row(5), Text{padded("")});
      CHECK_FALSE(reversed(2));
      CHECK(reversed(3)); // the selected item is highlighted
      CHECK_FALSE(reversed(4));
      SUBCASE("Escape closes without choosing") {
        send("\x1b");
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 1);
        CHECK_EQ(chosen, 0);
        CHECK_EQ(row(3), Text{padded("x")}); // the field is back
      }
      SUBCASE("Enter chooses") {
        send("\x1b[A"); // up
        CHECK(reversed(2));
        CHECK_FALSE(reversed(3));
        send("\x0d");
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 0);
        CHECK_EQ(chosen, 1);
        CHECK_EQ(row(1), Text{closed("Apple")});
        CHECK_EQ(row(3), Text{padded("x")});
      }
      SUBCASE("Space chooses") {
        send("\x1b[B");
        send(" ");
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 2);
        CHECK_EQ(chosen, 1);
      }
      SUBCASE("The highlight stops at the ends") {
        send("\x1b[B");
        send("\x1b[B");
        send("\x1b[B");
        CHECK(reversed(4));
        send("\x1b[H"); // home
        CHECK(reversed(2));
        send("\x1b[A");
        CHECK(reversed(2));
        send("\x1b[F"); // end
        CHECK(reversed(4));
      }
      SUBCASE("Other keys are not for the list") {
        send("X");
        CHECK(drop.isOpen());
        CHECK_EQ(field.content().data[0].size(), 1);
      }
      SUBCASE("Choosing the selected item is a choice too") {
        send("\x0d");
        CHECK_EQ(drop.selected(), 1);
        CHECK_EQ(chosen, 1);
      }
    }
    SUBCASE("Down opens") {
      send("\x1b[B");
      CHECK(drop.isOpen());
    }
    SUBCASE("Other keys do nothing when closed") {
      send("X");
      send("\x1b[A");
      CHECK_FALSE(drop.isOpen());
      CHECK_EQ(field.content().data[0].size(), 1);
    }
    SUBCASE("Mouse") {
      SUBCASE("Click opens, click on an item chooses") {
        send("\x1b[<0;4;2M"); // display (3, 1): the drop down
        CHECK(drop.isOpen());
        send("\x1b[<0;4;2m"); // released over it: stays open
        CHECK(drop.isOpen());
        send("\x1b[<0;4;5M"); // display (3, 4): "Cherry"
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 2);
        CHECK_EQ(chosen, 1);
        send("\x1b[<0;4;5m"); // the release goes nowhere
        CHECK_EQ(chosen, 1);
        CHECK_EQ(row(1), Text{closed("Cherry")});
        CHECK_EQ(field.content().data[0].size(), 1); // nothing has been typed into the field
      }
      SUBCASE("Clicking the drop down again closes") {
        send("\x1b[<0;4;2M");
        send("\x1b[<0;4;2m");
        send("\x1b[<0;4;2M");
        send("\x1b[<0;4;2m");
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(chosen, 0);
      }
      SUBCASE("Press, move to an item and release") {
        send("\x1b[<0;4;2M");
        send("\x1b[<32;4;3M"); // display (3, 2): "Apple"
        CHECK(reversed(2));
        CHECK_FALSE(reversed(3));
        send("\x1b[<32;4;5M"); // "Cherry"
        CHECK(reversed(4));
        send("\x1b[<0;4;5m");
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 2);
        CHECK_EQ(chosen, 1);
      }
      SUBCASE("Press and release elsewhere closes") {
        send("\x1b[<0;4;2M");
        send("\x1b[<32;11;9M");
        send("\x1b[<0;11;9m"); // display (10, 8)
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 1);
        CHECK_EQ(chosen, 0);
      }
      SUBCASE("Motion over the open list highlights") {
        send(" ");
        send("\x1b[<35;4;3M"); // motion without a button over "Apple"
        CHECK(reversed(2));
        CHECK_FALSE(reversed(3));
        send("\x1b[<35;4;2M"); // over the drop down itself: nothing changes
        CHECK(reversed(2));
      }
      SUBCASE("Wheel") {
        send(" ");
        send("\x1b[<65;4;3M"); // wheel down over the list
        CHECK(reversed(4));
        send("\x1b[<64;4;3M"); // wheel up
        CHECK(reversed(3));
        send("\x1b[<64;4;3M");
        CHECK(reversed(2));
        CHECK(drop.isOpen());
        CHECK_EQ(chosen, 0);
      }
      SUBCASE("Wheel does not open or click") {
        send("\x1b[<64;4;2M");
        send("\x1b[<65;4;2M");
        CHECK_FALSE(drop.isOpen());
      }
      SUBCASE("The other buttons do not do anything") {
        send("\x1b[<2;4;2M");
        send("\x1b[<2;4;2m");
        CHECK_FALSE(drop.isOpen());
      }
      SUBCASE("Clicking the frame closes but keeps the focus") {
        send(" ");
        send("\x1b[<0;1;1M"); // display (0, 0)
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(w1.focus(), &drop);
        CHECK_EQ(row(3), Text{padded("x")});
      }
      SUBCASE("Clicking another widget closes and moves the focus") {
        send(" ");
        send("\x1b[<0;4;7M"); // display (3, 6): field2
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(w1.focus(), &field2);
        CHECK_EQ(drop.selected(), 1);
        CHECK_EQ(chosen, 0);
        send("Z");
        CHECK_EQ(field2.content().data[0].size(), 2);
      }
    }
    SUBCASE("Scrolling") {
      std::vector<Text> many;
      for (auto i = 0; i < 12; ++i) {
        many.emplace_back("Item" + std::to_string(i));
      }
      drop.items(many);
      drop.maxRows(3);
      drop.select(0);
      CHECK_EQ(drop.maxRows(), 3);
      send(" ");
      CHECK_EQ(row(2), Text{padded(" Item0")});
      CHECK_EQ(row(4), Text{padded(" Item2")});
      CHECK_EQ(row(5), Text{padded("")}); // only three rows
      send("\x1b[F");
      CHECK_EQ(row(2), Text{padded(" Item9")});
      CHECK_EQ(row(4), Text{padded(" Item11")});
      CHECK(reversed(4));
      send("\x1b[5~"); // page up
      CHECK_EQ(row(2), Text{padded(" Item8")});
      CHECK(reversed(2));
      send("\x1b[<0;4;4M"); // display (3, 3): the middle row, "Item9"
      CHECK_EQ(drop.selected(), 9);
    }
    SUBCASE("The list closes") {
      send(" ");
      REQUIRE(drop.isOpen());
      SUBCASE("When the window loses the focus") {
        auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{22, 3, 38, 8}, Text{"Other"})};
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(row(3), Text{padded("x")});
      }
      SUBCASE("When the focus moves to another widget") {
        w1.focus(&field);
        CHECK_FALSE(drop.isOpen());
      }
      SUBCASE("When the drop down is moved") {
        drop.move(Rectangle{2, 1, 18, 2});
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.bounds(), Rectangle{2, 1, 18, 2});
        CHECK_EQ(row(3), Text{padded("x")});
      }
      SUBCASE("When the items are replaced") {
        drop.items({Text{"Only"}});
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(drop.selected(), 0);
        CHECK_EQ(row(1), Text{closed("Only")});
      }
      SUBCASE("When the window comes back after another was in front") {
        auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{22, 3, 38, 8}, Text{"Other"})};
        send("\x1b[<0;4;2M"); // display (3, 1): back to w1
        CHECK_EQ(term.focus(), &w1);
        CHECK(drop.isOpen()); // this click opened it again
      }
    }
    SUBCASE("Destroyed while open") {
      {
        auto temp{jwezel::ui::DropDown(w1, Rectangle{1, 8, 19, 9}, fruit)};
        w1.focus(&temp);
        temp.open();
        CHECK(temp.isOpen());
        CHECK_EQ(row(9), Text{padded(" Apple")});
      }
      CHECK_EQ(w1.focus(), nullptr);
      CHECK_EQ(row(9), Text{padded("")});
      CHECK_EQ(row(8), Text{padded("")});
      send("\x1b[<0;4;10M"); // where the popup was: nothing
      send("\x1b[<0;4;10m");
    }
    SUBCASE("No items") {
      drop.items({});
      CHECK_EQ(drop.selected(), -1);
      CHECK_EQ(row(1), Text{closed("")});
      send(" ");
      CHECK_FALSE(drop.isOpen());
    }
    SUBCASE("Selecting") {
      drop.select(2);
      CHECK_EQ(row(1), Text{closed("Cherry")});
      drop.select(7);
      CHECK_EQ(drop.selected(), 2);
      drop.select(-4);
      CHECK_EQ(drop.selected(), 0);
      CHECK_EQ(chosen, 0); // programmatic selection is not a choice
    }
    SUBCASE("In a box") {
      auto box{jwezel::ui::VerticalBox(w1, Rectangle{1, 8, 19, 11})};
      auto first{jwezel::ui::DropDown(box, fruit, 0)};
      auto second{jwezel::ui::DropDown(box, fruit, 2)};
      CHECK_EQ(first.bounds(), Rectangle{1, 8, 19, 10});
      CHECK_EQ(second.bounds(), Rectangle{1, 10, 19, 11});
      CHECK_EQ(row(8), Text{closed("Apple")});
      CHECK_EQ(row(10), Text{closed("Cherry")});
      w1.focus(&second);
      send(" ");
      CHECK(second.isOpen());
      CHECK_EQ(row(11), Text{padded(" Apple")}); // opens below the second one
    }
  }

  SUBCASE("Window decoration") {
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    auto shown{[&](const Rectangle &area) {return term.display().text()[area];}};
    const Rectangle area1{0, 0, 20, 12}, area2{22, 0, 34, 6};
    SUBCASE("The state follows the focus") {
      Recording recording;
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"One"}, &recording)};
      CHECK_EQ(w1.state(), DecorationState::active); // a new window has the focus
      CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{"One"}));
      auto w2{jwezel::ui::Window(ui_, ' '_C, area2, Text{"Two"})};
      CHECK_EQ(w2.state(), DecorationState::active);
      CHECK_EQ(w1.state(), DecorationState::inactive); // and the one that had it has not anymore
      CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{"One"}, true));
      CHECK_EQ(shown(area2), framed(Vector{12, 6}, Text{"Two"}));
      send("\x1b[<0;1;1M"); // click on the frame of the first
      send("\x1b[<0;1;1m");
      CHECK_EQ(w1.state(), DecorationState::active);
      CHECK_EQ(w2.state(), DecorationState::inactive);
      CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{"One"}));
      CHECK_EQ(shown(area2), framed(Vector{12, 6}, Text{"Two"}, true));
      // Only the changes were drawn: once for creation, once for losing the focus, once for getting it back
      using enum DecorationState;
      CHECK_EQ(recording.states, std::vector<DecorationState>{active, inactive, active});
      send("\x1b[<0;2;2M"); // on the content of the focused window: nothing changes
      send("\x1b[<0;2;2m");
      CHECK_EQ(recording.states.size(), 3);
    }
    SUBCASE("The state is back when a window is destroyed and the focus goes to the desktop") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"One"})};
      {
        auto w2{jwezel::ui::Window(ui_, ' '_C, area2, Text{"Two"})};
        CHECK_EQ(w1.state(), DecorationState::inactive);
      }
      CHECK_EQ(term.focus(), &term.desktop()); // the desktop gets it, not the window below
      CHECK_EQ(w1.state(), DecorationState::inactive);
      send("\x1b[<0;2;2M"); // a click makes it active
      send("\x1b[<0;2;2m");
      CHECK_EQ(w1.state(), DecorationState::active);
    }
    SUBCASE("A decoration without a frame") {
      NoFrame none;
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"One"}, &none)};
      CHECK_EQ(w1.content(), Rectangle{0, 0, 20, 12}); // the content has all the room
      CHECK_EQ(shown(area1), Text{' '_C, Vector{20, 12}});
      auto box{jwezel::ui::VerticalBox(w1, jwezel::RectangleDefault)};
      auto label{jwezel::ui::Label(box, Text{"Hi"})};
      CHECK_EQ(label.bounds(), Rectangle{0, 0, 20, 12});
      SUBCASE("It can be changed while the window is there") {
        w1.decoration(nullptr);
        CHECK_EQ(&w1.decoration(), &jwezel::ui::defaultDecoration());
        CHECK_EQ(w1.content(), Rectangle{1, 1, 19, 11}); // the content has made room
        CHECK_EQ(label.bounds(), Rectangle{1, 1, 19, 11});
        CHECK_EQ(w1.text(), framed(Vector{20, 12}, Text{"One"}));
        w1.decoration(&none); // and back, which takes the frame away
        CHECK_EQ(label.bounds(), Rectangle{0, 0, 20, 12});
        CHECK_EQ(w1.text(), Text{' '_C, Vector{20, 12}});
        auto top{Text{' '_C, Vector{20, 1}}};
        top.patch(Text{"Hi"}, Vector{0, 0}); // only the label is in the top line now
        CHECK_EQ(shown(area1)[Rectangle{0, 0, 20, 1}], top);
      }
    }
    SUBCASE("A decoration chosen for the window") {
      jwezel::ui::BoxDecoration heavy{jwezel::ui::BorderStyle{2, 0, false}, jwezel::ui::BorderStyle{2, 1, false}};
      auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 8, 4}, Text{"Hi"}, &heavy)};
      CHECK_EQ(std::string(shown(Rectangle{0, 0, 8, 4})), "┏Hi━━━━┓\n┃      ┃\n┃      ┃\n┗━━━━━━┛\n");
      auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{10, 0, 18, 4}, Text{"Yo"}, &heavy)};
      CHECK_EQ(std::string(shown(Rectangle{0, 0, 8, 4})), "┏Hi╍╍╍╍┓\n╏      ╏\n╏      ╏\n┗╍╍╍╍╍╍┛\n");
    }
    SUBCASE("Title") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"A long title"})};
      CHECK_EQ(w1.title(), Text{"A long title"});
      w1.title(Text{"Short"});
      CHECK_EQ(w1.title(), Text{"Short"});
      CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{"Short"})); // nothing remains of the old one
      w1.title(Text{});
      CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{}));
    }
    SUBCASE("Colors") {
      const auto cell{[&](jwezel::Dim x, jwezel::Dim y) {return term.display().text().data[static_cast<size_t>(y)][static_cast<size_t>(x)];}};
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"One"}, nullptr, &paint)};
      CHECK_EQ(&w1.colors(), &paint);
      CHECK_EQ(cell(0, 0).attributes.fg, jwezel::RgbBlue); // frame
      CHECK_EQ(cell(5, 11).attributes.fg, jwezel::RgbBlue);
      CHECK_EQ(cell(1, 0).attributes.fg, jwezel::RgbYellow); // title
      CHECK_EQ(cell(1, 0).attributes.attr, jwezel::bold);
      CHECK_EQ(cell(5, 0).attributes.attr, 0);
      CHECK_EQ(cell(1, 0).rune, 'O');
      CHECK_EQ(cell(5, 5).attributes.fg, jwezel::RgbNone); // content is not touched
      auto w2{jwezel::ui::Window(ui_, ' '_C, area2, Text{"Two"}, nullptr, &paint)};
      CHECK_EQ(cell(0, 0).attributes.fg, jwezel::RgbGray5); // inactive
      CHECK_EQ(cell(1, 0).attributes.fg, jwezel::RgbGray5);
      CHECK_EQ(cell(1, 0).attributes.attr, jwezel::underline); // the bold is gone
      CHECK_EQ(cell(22, 0).attributes.fg, jwezel::RgbBlue); // the other window
      send("\x1b[<0;1;1M"); // back to the first
      send("\x1b[<0;1;1m");
      CHECK_EQ(cell(0, 0).attributes.fg, jwezel::RgbBlue);
      CHECK_EQ(cell(1, 0).attributes.attr, jwezel::bold);
      CHECK_EQ(cell(22, 0).attributes.fg, jwezel::RgbGray5);
      SUBCASE("The style can be changed") {
        w1.colors(nullptr);
        CHECK_EQ(&w1.colors(), &jwezel::ui::defaultColorStyle());
        CHECK_EQ(cell(0, 0).attributes.fg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 0).attributes.attr, 0);
        CHECK_EQ(shown(area1), framed(Vector{20, 12}, Text{"One"}));
        w1.colors(&paint);
        CHECK_EQ(cell(0, 0).attributes.fg, jwezel::RgbBlue);
      }
      SUBCASE("The background shows through the frame") {
        auto w3{jwezel::ui::Window(ui_, Char{' ', jwezel::RgbNone, jwezel::RgbGray2}, Rectangle{22, 8, 34, 14}, Text{"Bg"}, nullptr, &paint)};
        CHECK_EQ(cell(22, 8).attributes.fg, jwezel::RgbBlue);
        CHECK_EQ(cell(22, 8).attributes.bg, jwezel::RgbGray2);
        CHECK_EQ(cell(23, 8).attributes.bg, jwezel::RgbGray2); // title
        CHECK_EQ(cell(25, 11).attributes.bg, jwezel::RgbGray2); // inside
      }
    }
    SUBCASE("Resizing an inactive window") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"One"})};
      auto w2{jwezel::ui::Window(ui_, ' '_C, area2, Text{"Two"})};
      REQUIRE_EQ(w1.state(), DecorationState::inactive);
      w1.move(Rectangle{0, 0, 16, 9});
      CHECK_EQ(shown(Rectangle{0, 0, 16, 9}), framed(Vector{16, 9}, Text{"One"}, true));
      w1.move(Rectangle{0, 0, 21, 14});
      CHECK_EQ(shown(Rectangle{0, 0, 21, 14}), framed(Vector{21, 14}, Text{"One"}, true));
      CHECK_EQ(w1.text(), framed(Vector{21, 14}, Text{"One"}, true));
    }
    SUBCASE("Decorated windows and frames on the desktop") {
      auto &desktop{ui_.desktop()};
      auto frame{jwezel::ui::Frame(desktop, Rectangle{2, 1, 12, 5}, ' '_C, nullptr, nullptr, Text{"F"})};
      CHECK_EQ(frame.state(), DecorationState::inactive);
      CHECK_EQ(shown(Rectangle{2, 1, 12, 5}), framed(Vector{10, 4}, Text{"F"}, true));
      frame.state(DecorationState::active);
      CHECK_EQ(shown(Rectangle{2, 1, 12, 5}), framed(Vector{10, 4}, Text{"F"}));
      frame.title(Text{"Frame"});
      CHECK_EQ(shown(Rectangle{2, 1, 12, 5}), framed(Vector{10, 4}, Text{"Frame"}));
      frame.move(Rectangle{3, 2, 9, 6}); // resized: nothing of the old frame remains
      CHECK_EQ(shown(Rectangle{3, 2, 9, 6}), framed(Vector{6, 4}, Text{"Fram"})); // the title is clipped
      CHECK_EQ(shown(Rectangle{9, 1, 12, 5}), Text{'.'_C, Vector{3, 4}}); // the desktop is back where the frame was
      NoFrame none;
      auto bare{jwezel::ui::Frame(desktop, Rectangle{14, 1, 20, 5}, 'x'_C, &none)};
      CHECK_EQ(shown(Rectangle{14, 1, 20, 5}), Text{'x'_C, Vector{6, 4}});
    }
  }

  SUBCASE("Widget styles") {
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    auto click{
      [&](int x, int y) { // display position
        send(std::format("\x1b[<0;{};{}M", x + 1, y + 1).c_str());
        send(std::format("\x1b[<0;{};{}m", x + 1, y + 1).c_str());
      }
    };
    const auto cell{
      [&](jwezel::Dim x, jwezel::Dim y) {return term.display().text().data[static_cast<size_t>(y)][static_cast<size_t>(x)];}
    };
    auto shown{[&](const Rectangle &area) {return term.display().text()[area];}};
    const Rectangle area1{0, 0, 24, 14};
    const std::vector<Text> fruit{Text{"Apple"}, Text{"Banana"}, Text{"Cherry"}};
    SUBCASE("Colors") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"T"}, nullptr, &widgetColors)};
      auto button1{jwezel::ui::Button(w1, Rectangle{1, 1, 9, 2}, Text{"OK"})};
      auto button2{jwezel::ui::Button(w1, Rectangle{1, 3, 9, 4}, Text{"No"})};
      auto input{jwezel::ui::LineEdit(w1, Rectangle{1, 5, 19, 6}, Text{"ab"})};
      auto label{jwezel::ui::Label(w1, Rectangle{1, 7, 10, 8}, Text{"Hi"})};
      auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 9, 19, 10}, fruit)};
      REQUIRE_EQ(w1.focus(), &button1);
      CHECK_EQ(&w1.colors(), &widgetColors);
      SUBCASE("Roles of the normal and the focused state") {
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbGreen); // button.focus, on the whole button
        CHECK_EQ(cell(8, 1).attributes.fg, jwezel::RgbGreen);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbBlue); // button
        CHECK_EQ(cell(18, 5).attributes.bg, jwezel::RgbGray2); // input, also where there is nothing
        CHECK_EQ(cell(1, 5).attributes.bg, jwezel::RgbGray2); // and under the content
        CHECK_EQ(cell(1, 5).rune, 'a');
        CHECK_EQ(cell(1, 7).attributes.fg, jwezel::RgbYellow); // label
        CHECK_EQ(cell(2, 7).rune, 'i');
        CHECK_EQ(cell(2, 7).attributes.fg, jwezel::RgbYellow);
        CHECK_EQ(cell(5, 7).attributes.fg, jwezel::RgbYellow); // the whole widget
        CHECK_EQ(cell(1, 9).attributes.fg, jwezel::RgbCyan); // dropdown
        CHECK_EQ(cell(17, 9).attributes.fg, jwezel::RgbCyan);
        CHECK_EQ(cell(12, 12).attributes.fg, jwezel::RgbNone); // no widget there
      }
      SUBCASE("The focus follows") {
        click(2, 3);
        CHECK_EQ(w1.focus(), &button2);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbGreen);
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbBlue);
        click(2, 5);
        CHECK_EQ(cell(18, 5).attributes.bg, jwezel::RgbGray5); // input.focus
        CHECK_EQ(cell(1, 5).attributes.bg, jwezel::RgbGray5);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbBlue);
        send("X");
        CHECK_EQ(cell(3, 5).rune, 'X'); // typed characters are in the field as well
        CHECK_EQ(cell(3, 5).attributes.bg, jwezel::RgbGray5);
        click(2, 3);
        CHECK_EQ(cell(18, 5).attributes.bg, jwezel::RgbGray2);
      }
      SUBCASE("A pressed button is in use") {
        send("\x1b[<0;3;4M"); // down on button2
        CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbRed); // button.active
        CHECK_NE(cell(1, 3).attributes.attr & jwezel::reverse, 0);
        CHECK_EQ(cell(8, 3).attributes.bg, jwezel::RgbRed);
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbBlue); // the focus is on button2 now
        send("\x1b[<0;3;4m"); // up: nothing of the active role is left
        CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 3).attributes.attr, 0);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbGreen); // focused
        send("\x1b[<0;3;4M"); // pressed again, then called off
        CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbRed);
        send("\x1b[<32;15;12M");
        send("\x1b[<0;15;12m");
        CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 3).attributes.attr, 0);
      }
      SUBCASE("A window that has not the focus has no focused widget") {
        auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{26, 0, 38, 6}, Text{"B"})};
        CHECK_EQ(w1.focus(), &button1); // still the focused widget of its window
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbBlue); // but it looks like the others
        click(0, 0); // the frame of w1
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbGreen);
      }
      SUBCASE("The drop down") {
        click(2, 9); // opens it
        REQUIRE(drop.isOpen());
        CHECK_EQ(cell(1, 9).attributes.fg, jwezel::RgbMagenta); // dropdown.active
        CHECK_EQ(cell(10, 11).attributes.bg, jwezel::RgbGray3); // popup
        CHECK_EQ(cell(10, 12).attributes.bg, jwezel::RgbGray3);
        CHECK_EQ(cell(5, 10).attributes.bg, jwezel::RgbBlue5); // popup.highlight, on the highlighted item
        CHECK_EQ(cell(5, 10).attributes.fg, jwezel::RgbWhite);
        CHECK_EQ(cell(5, 10).attributes.attr, jwezel::bold);
        CHECK_EQ(cell(17, 10).attributes.bg, jwezel::RgbBlue5); // across the whole row
        send("\x1b[B"); // the highlight moves
        CHECK_EQ(cell(5, 11).attributes.bg, jwezel::RgbBlue5);
        CHECK_EQ(cell(5, 10).attributes.bg, jwezel::RgbGray3); // and leaves nothing behind
        CHECK_EQ(cell(5, 10).attributes.attr, 0);
        CHECK_EQ(cell(5, 10).attributes.fg, jwezel::RgbNone);
        send("\x1b"); // closed
        CHECK_FALSE(drop.isOpen());
        CHECK_EQ(cell(1, 9).attributes.fg, jwezel::RgbCyan); // "dropdown.focus" is not there: "dropdown"
      }
      SUBCASE("Changing the color style draws everything again") {
        w1.colors(nullptr);
        CHECK_EQ(&w1.colors(), &jwezel::ui::defaultColorStyle());
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbNone);
        CHECK_EQ(cell(18, 5).attributes.bg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 7).attributes.fg, jwezel::RgbNone);
        CHECK_EQ(cell(1, 9).attributes.fg, jwezel::RgbNone);
        CHECK_EQ(std::string(shown(Rectangle{1, 1, 7, 2})), "< OK >\n"); // the glyphs are not colors
        w1.colors(&widgetColors);
        CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbGreen);
        CHECK_EQ(cell(1, 3).attributes.fg, jwezel::RgbBlue);
        CHECK_EQ(cell(18, 5).attributes.bg, jwezel::RgbGray2);
      }
    }
    SUBCASE("Nested widgets are drawn again") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"T"})};
      auto outer{jwezel::ui::VerticalBox(w1, jwezel::RectangleDefault)};
      auto inner{jwezel::ui::HorizontalBox(outer, jwezel::RectangleDefault)};
      auto button{jwezel::ui::Button(inner, Text{"OK"})};
      CHECK_EQ(cell(2, 1).attributes.fg, jwezel::RgbNone);
      w1.colors(&widgetColors);
      CHECK_EQ(cell(2, 1).attributes.fg, jwezel::RgbGreen); // it is the focused one
    }
    SUBCASE("The colors of the desktop") {
      auto &desktop{ui_.desktop()};
      auto button{jwezel::ui::Button(desktop, Rectangle{2, 1, 10, 2}, Text{"Go"})};
      CHECK_EQ(cell(2, 1).attributes.fg, jwezel::RgbNone);
      desktop.colors(&widgetColors);
      CHECK_EQ(cell(2, 1).attributes.fg, jwezel::RgbGreen);
      auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{14, 0, 30, 6}, Text{"T"})}; // has its own colors
      auto inner{jwezel::ui::Button(w1, Rectangle{1, 1, 9, 2}, Text{"In"})};
      CHECK_EQ(cell(15, 1).attributes.fg, jwezel::RgbNone);
      CHECK_EQ(cell(2, 1).attributes.fg, jwezel::RgbBlue); // the desktop has not the focus now
    }
    SUBCASE("Glyphs") {
      auto w1{jwezel::ui::Window(ui_, ' '_C, area1, Text{"T"}, nullptr, &plain, &roundStyle)};
      auto button1{jwezel::ui::Button(w1, Rectangle{1, 1, 9, 2}, Text{"OK"})};
      auto button2{jwezel::ui::Button(w1, Rectangle{1, 3, 9, 4}, Text{"No"})};
      auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 9, 19, 10}, fruit, 1)};
      CHECK_EQ(&w1.style(), &roundStyle);
      CHECK_EQ(shown(Rectangle{1, 1, 7, 2}), Text{"< OK >"}); // by state: focused
      CHECK_EQ(shown(Rectangle{1, 3, 7, 4}), Text{"( No )"}); // normal
      SUBCASE("A pressed button") {
        send("\x1b[<0;3;4M");
        CHECK_EQ(shown(Rectangle{1, 3, 7, 4}), Text{"{ No }"});
        CHECK_EQ(shown(Rectangle{1, 1, 7, 2}), Text{"( OK )"});
        send("\x1b[<0;3;4m");
        CHECK_EQ(shown(Rectangle{1, 3, 7, 4}), Text{"< No >"});
      }
      SUBCASE("The drop down") {
        const auto plain{[&](const Rectangle &area) {return std::string(shown(area));}}; // without the highlight
        CHECK_EQ(plain(Rectangle{1, 9, 19, 10}), "( Banana       v )\n");
        click(2, 9);
        REQUIRE(drop.isOpen());
        CHECK_EQ(plain(Rectangle{1, 9, 19, 10}), "( Banana       ^ )\n"); // open
        CHECK_EQ(plain(Rectangle{1, 10, 19, 13}), "|Apple           |\n|Banana          |\n|Cherry          |\n");
        send("\x1b");
        CHECK_EQ(plain(Rectangle{1, 9, 19, 10}), "( Banana       v )\n");
      }
      SUBCASE("The popup is as wide as the glyphs and the items need") {
        drop.items({Text{"A very long item of the list"}, Text{"B"}});
        click(2, 9);
        REQUIRE(drop.isOpen());
        CHECK_EQ(std::string(shown(Rectangle{1, 10, 31, 11})), "|A very long item of the list|\n"); // 28 + 2
        CHECK_EQ(std::string(shown(Rectangle{1, 11, 31, 12})), "|B                           |\n");
      }
      SUBCASE("The glyphs of the popup can have any width") {
        w1.style(&wideStyle);
        drop.items({Text{"A very long item of the list"}, Text{"B"}});
        click(2, 9);
        REQUIRE(drop.isOpen());
        // 2 + 28 + 3 wide: nothing of the item is cut off, nor is the glyph on the right
        CHECK_EQ(std::string(shown(Rectangle{1, 10, 34, 11})), "<<A very long item of the list>>>\n");
        CHECK_EQ(std::string(shown(Rectangle{1, 11, 34, 12})), "<<B                           >>>\n");
        CHECK_EQ(std::string(shown(Rectangle{1, 9, 19, 10})), "[ B            ▼ ]\n"); // default drop down glyphs
      }
      SUBCASE("The style can be changed") {
        w1.style(nullptr);
        CHECK_EQ(&w1.style(), &jwezel::ui::defaultUiStyle());
        CHECK_EQ(shown(Rectangle{1, 1, 7, 2}), Text{"< OK >"}); // the default glyph of a focused button
        CHECK_EQ(shown(Rectangle{1, 3, 7, 4}), Text{"[ No ]"});
        CHECK_EQ(shown(Rectangle{1, 9, 19, 10}), Text{"[ Banana       ▼ ]"});
        w1.style(&roundStyle);
        CHECK_EQ(shown(Rectangle{1, 1, 7, 2}), Text{"< OK >"});
      }
    }
    SUBCASE("The decoration is part of the style") {
      jwezel::ui::BoxDecoration heavy{jwezel::ui::BorderStyle{2, 0, false}, jwezel::ui::BorderStyle{2, 0, false}};
      NoFrame none;
      const jwezel::ui::UiStyle heavyStyle{.decoration = &heavy};
      const Rectangle area{0, 0, 8, 4};
      auto w1{jwezel::ui::Window(ui_, ' '_C, area, Text{"Hi"}, nullptr, nullptr, &heavyStyle)};
      CHECK_EQ(&w1.decoration(), &heavy);
      CHECK_EQ(std::string(shown(area)), "┏Hi━━━━┓\n┃      ┃\n┃      ┃\n┗━━━━━━┛\n");
      SUBCASE("A decoration of the window itself comes first") {
        w1.decoration(&none);
        CHECK_EQ(&w1.decoration(), &none);
        CHECK_EQ(w1.content(), Rectangle{0, 0, 8, 4});
        w1.decoration(nullptr); // the one of the style again
        CHECK_EQ(&w1.decoration(), &heavy);
      }
      SUBCASE("The style can be changed") {
        auto box{jwezel::ui::VerticalBox(w1, jwezel::RectangleDefault)};
        auto label{jwezel::ui::Label(box, Text{"x"})};
        const jwezel::ui::UiStyle noFrameStyle{.decoration = &none};
        w1.style(&noFrameStyle);
        CHECK_EQ(w1.content(), Rectangle{0, 0, 8, 4}); // the content has made room
        CHECK_EQ(label.bounds(), Rectangle{0, 0, 8, 4});
        CHECK_EQ(w1.text(), Text{' '_C, Vector{8, 4}});
        w1.style(nullptr);
        CHECK_EQ(&w1.decoration(), &jwezel::ui::defaultDecoration());
        CHECK_EQ(label.bounds(), Rectangle{1, 1, 7, 3});
        CHECK_EQ(w1.text(), framed(Vector{8, 4}, Text{"Hi"}));
      }
    }
  }

  SUBCASE("Focus tint") {
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    const auto cell{
      [&](jwezel::Dim x, jwezel::Dim y) {return term.display().text().data[static_cast<size_t>(y)][static_cast<size_t>(x)];}
    };
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 24, 12}, Text{"T"})}; // the default colors
    auto input1{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 19, 2}, Text{"One"})};
    auto input2{jwezel::ui::TextArea(w1, Rectangle{1, 3, 19, 5}, Text{"Two"})};
    auto button{jwezel::ui::Button(w1, Rectangle{1, 6, 9, 7}, Text{"OK"})};
    auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 8, 19, 9}, std::vector<Text>{Text{"Apple"}})};
    REQUIRE_EQ(w1.focus(), &input1);
    SUBCASE("The field with the focus is tinted all over") {
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbGray5); // under the text
      CHECK_EQ(cell(1, 1).rune, 'O');
      CHECK_EQ(cell(18, 1).attributes.bg, jwezel::RgbGray5); // and where there is none
      CHECK_EQ(cell(1, 1).attributes.fg, jwezel::RgbNone); // nothing but the background changes
      CHECK_EQ(cell(1, 1).attributes.attr, 0);
      CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbNone); // other fields are not
      CHECK_EQ(cell(18, 4).attributes.bg, jwezel::RgbNone);
      CHECK_EQ(cell(20, 1).attributes.bg, jwezel::RgbNone); // nor is what is next to it
    }
    SUBCASE("It goes where the focus goes") {
      send("\x1b[<0;3;4M"); // down on the text area
      send("\x1b[<0;3;4m");
      REQUIRE_EQ(w1.focus(), &input2);
      CHECK_EQ(cell(1, 3).attributes.bg, jwezel::RgbGray5);
      CHECK_EQ(cell(18, 4).attributes.bg, jwezel::RgbGray5); // all lines of it
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbNone);
      send("X");
      CHECK_EQ(cell(4, 3).rune, 'X');
      CHECK_EQ(cell(4, 3).attributes.bg, jwezel::RgbGray5); // typed characters are in the tinted field
    }
    SUBCASE("A button shows the focus in its brackets and is tinted as well") {
      CHECK_EQ(cell(1, 6).rune, '['); // not the focused widget
      CHECK_EQ(cell(1, 6).attributes.bg, jwezel::RgbNone);
      send("\x1b[<0;3;7M"); // down on the button
      REQUIRE_EQ(w1.focus(), &button);
      CHECK_EQ(cell(1, 6).rune, '<'); // pressed, which is the focused widget as well
      CHECK_EQ(cell(6, 6).rune, '>');
      CHECK_EQ(cell(1, 6).attributes.bg, jwezel::RgbGray5);
      send("\x1b[<0;3;7m");
      CHECK_EQ(std::string(term.display().text()[Rectangle{1, 6, 7, 7}]), "< OK >\n");
      for (jwezel::Dim x = 1; x < 9; ++x) { // the whole button, and nothing else
        CHECK_EQ(cell(x, 6).attributes.bg, jwezel::RgbGray5);
      }
      CHECK_EQ(cell(9, 6).attributes.bg, jwezel::RgbNone);
      CHECK_EQ(cell(1, 6).attributes.fg, jwezel::RgbNone);
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbNone); // and the field is not anymore
      CHECK_EQ(cell(1, 1).rune, 'O');
      send("\x1b[<0;3;2M"); // the focus goes back to the field
      send("\x1b[<0;3;2m");
      CHECK_EQ(std::string(term.display().text()[Rectangle{1, 6, 7, 7}]), "[ OK ]\n");
      CHECK_EQ(cell(1, 6).attributes.bg, jwezel::RgbNone);
    }
    SUBCASE("A drop down is tinted") {
      send("\x1b[<0;3;9M"); // down on it: opens
      REQUIRE_EQ(w1.focus(), &drop);
      REQUIRE(drop.isOpen());
      CHECK_EQ(cell(1, 8).rune, '['); // there is no other glyph for the focus
      for (jwezel::Dim x = 1; x < 19; ++x) { // also while it is open
        CHECK_EQ(cell(x, 8).attributes.bg, jwezel::RgbGray5);
      }
      CHECK_EQ(cell(10, 9).attributes.bg, jwezel::RgbNone); // but not the list
      send("\x1b[<0;3;9m");
      send("\x1b"); // closed
      REQUIRE_FALSE(drop.isOpen());
      CHECK_EQ(cell(1, 8).attributes.bg, jwezel::RgbGray5);
      CHECK_EQ(cell(18, 8).attributes.bg, jwezel::RgbGray5);
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbNone);
      send("\x1b[<0;3;2M"); // the field has the focus
      send("\x1b[<0;3;2m");
      CHECK_EQ(cell(1, 8).attributes.bg, jwezel::RgbNone);
    }
    SUBCASE("A window without the focus does not advertise one") {
      auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{26, 0, 38, 6}, Text{"B"})};
      REQUIRE_EQ(w1.focus(), &input1); // still the focused field of its window
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbNone);
      CHECK_EQ(cell(18, 1).attributes.bg, jwezel::RgbNone);
      send("\x1b[<0;1;1M"); // back to the first window
      send("\x1b[<0;1;1m");
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbGray5);
    }
    SUBCASE("A theme decides") {
      w1.colors(&plain); // without a role for it
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbNone);
      w1.colors(&widgetColors); // with its own
      CHECK_EQ(cell(1, 1).attributes.bg, jwezel::RgbGray5);
      CHECK_EQ(cell(18, 1).attributes.bg, jwezel::RgbGray5);
    }
  }

  SUBCASE("Focus tint on a window with colors") {
    const auto cell{
      [&](jwezel::Dim x, jwezel::Dim y) {return term.display().text().data[static_cast<size_t>(y)][static_cast<size_t>(x)];}
    };
    // The tint is mixed into the background: it makes it lighter, it does not replace it
    const Char blue{' ', jwezel::RgbNone, jwezel::RgbBlue5};
    auto w1{jwezel::ui::Window(ui_, blue, Rectangle{0, 0, 24, 8}, Text{"T"})};
    auto input1{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 19, 2}, Text{"One"}, blue)};
    auto input2{jwezel::ui::LineEdit(w1, Rectangle{1, 3, 19, 4}, Text{"Two"}, blue)};
    CHECK_EQ(cell(18, 1).attributes.bg, jwezel::Rgb(0.25, 0.25, 0.5)); // between blue and gray
    CHECK_EQ(cell(18, 3).attributes.bg, jwezel::RgbBlue5);
    CHECK_EQ(cell(5, 5).attributes.bg, jwezel::RgbBlue5); // the window
  }

  SUBCASE("Keyboard navigation") {
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    auto click{
      [&](int x, int y) {
        send(std::format("\x1b[<0;{};{}M", x + 1, y + 1).c_str());
        send(std::format("\x1b[<0;{};{}m", x + 1, y + 1).c_str());
      }
    };
    const std::string tab{"\t"}, shiftTab{"\x1b[Z"}, enter{"\r"}, escape{"\x1b"};
    const std::vector<Text> fruit{Text{"Apple"}, Text{"Banana"}};
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 34, 14}, Text{"T"}, nullptr, &plain)};
    auto label{jwezel::ui::Label(w1, Rectangle{1, 1, 10, 2}, Text{"Name"})};
    auto entry{jwezel::ui::LineEdit(w1, Rectangle{1, 2, 20, 3}, Text{"a"})};
    auto button1{jwezel::ui::Button(w1, Rectangle{1, 4, 8, 5}, Text{"OK"})};
    auto area{jwezel::ui::TextArea(w1, Rectangle{1, 6, 20, 8}, Text{"x"})};
    auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 9, 20, 10}, fruit)};
    auto label2{jwezel::ui::Label(w1, Rectangle{22, 1, 30, 2}, Text{"Hi"})};
    auto button2{jwezel::ui::Button(w1, Rectangle{22, 4, 30, 5}, Text{"No"})};
    using jwezel::ui::FocusPolicy;
    using jwezel::ui::Widget;
    SUBCASE("The first widget that can have the focus has it") {
      CHECK_EQ(w1.focus(), &entry); // not the label that was created before it
      CHECK_EQ(label.focusPolicy(), FocusPolicy::none);
      CHECK_EQ(entry.focusPolicy(), FocusPolicy::strong);
      CHECK_EQ(w1.tabChain(), std::vector<Widget *>{&label, &entry, &button1, &area, &drop, &label2, &button2});
    }
    SUBCASE("Tab goes through the widgets in the order they were created") {
      for (Widget *expected: std::vector<Widget *>{&button1, &area, &drop, &button2, &entry, &button1}) { // wraps around
        send(tab.c_str());
        CHECK_EQ(w1.focus(), expected);
      }
    }
    SUBCASE("Shift-Tab goes the other way") {
      for (Widget *expected: std::vector<Widget *>{&button2, &drop, &area, &button1, &entry, &button2}) {
        send(shiftTab.c_str());
        CHECK_EQ(w1.focus(), expected);
      }
    }
    SUBCASE("Tab is not used up by the widgets that have text") {
      send(tab.c_str()); // button
      send(tab.c_str()); // the text area
      REQUIRE_EQ(w1.focus(), &area);
      send(tab.c_str());
      CHECK_EQ(w1.focus(), &drop);
      CHECK_EQ(area.content().data[0].size(), 1); // no Tab character was typed
      CHECK_EQ(entry.content().data[0].size(), 1);
    }
    SUBCASE("Moving the focus closes the list of a drop down") {
      click(2, 9);
      REQUIRE(drop.isOpen());
      send(tab.c_str());
      CHECK_FALSE(drop.isOpen());
      CHECK_EQ(w1.focus(), &button2);
    }
    SUBCASE("Labels do not get the focus by a click either") {
      click(2, 1);
      CHECK_EQ(w1.focus(), &entry);
      click(23, 4); // a button does
      CHECK_EQ(w1.focus(), &button2);
      click(2, 1);
      CHECK_EQ(w1.focus(), &button2);
      CHECK_EQ(term.focus(), &w1); // the window has it anyway
    }
    SUBCASE("Focus policy") {
      SUBCASE("Click only") {
        button1.focusPolicy(FocusPolicy::click);
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &area); // skipped
        click(2, 4);
        CHECK_EQ(w1.focus(), &button1); // but a click gets there
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &area);
      }
      SUBCASE("Tab only") {
        area.focusPolicy(FocusPolicy::tab);
        click(2, 6);
        CHECK_EQ(w1.focus(), &entry); // a click does not
        send(tab.c_str());
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &area); // Tab does
      }
      SUBCASE("None") {
        area.focusPolicy(FocusPolicy::none);
        click(2, 6);
        CHECK_EQ(w1.focus(), &entry); // a click does not give it the focus
        for (Widget *expected: std::vector<Widget *>{&button1, &drop, &button2, &entry}) { // and Tab skips it
          send(tab.c_str());
          CHECK_EQ(w1.focus(), expected);
        }
      }
      SUBCASE("The policy is about the focus, a widget can still be clicked") {
        drop.focusPolicy(FocusPolicy::none);
        click(2, 9);
        CHECK_EQ(w1.focus(), &entry);
        CHECK(drop.isOpen());
        send(tab.c_str()); // the focus moves on, and the list stays: nothing tells the drop down
        CHECK_EQ(w1.focus(), &button1);
      }
      SUBCASE("A widget that cannot have the focus anymore loses it") {
        entry.focusPolicy(FocusPolicy::none);
        CHECK_EQ(w1.focus(), nullptr);
        send(tab.c_str()); // from nowhere to the first
        CHECK_EQ(w1.focus(), &button1);
        entry.focusPolicy(FocusPolicy::strong); // does not take the focus away
        CHECK_EQ(w1.focus(), &button1);
      }
      SUBCASE("A widget that can have the focus gets it if there is none") {
        entry.focusPolicy(FocusPolicy::none);
        REQUIRE_EQ(w1.focus(), nullptr);
        entry.focusPolicy(FocusPolicy::tab);
        CHECK_EQ(w1.focus(), &entry);
      }
      SUBCASE("Nothing that can have the focus") {
        for (Widget *widget: std::vector<Widget *>{&entry, &button1, &area, &drop, &button2}) {
          widget->focusPolicy(FocusPolicy::none);
        }
        CHECK_EQ(w1.focus(), nullptr);
        send(tab.c_str()); // nothing to do, and nothing breaks
        send(shiftTab.c_str());
        CHECK_EQ(w1.focus(), nullptr);
      }
    }
    SUBCASE("Order") {
      SUBCASE("Two widgets") {
        w1.tabOrder(button2, entry); // the entry comes after the second button now
        CHECK_EQ(w1.tabChain(), std::vector<Widget *>{&label, &button1, &area, &drop, &label2, &button2, &entry});
        send(tab.c_str()); // from the entry, which is last now, to the first
        CHECK_EQ(w1.focus(), &button1);
        send(shiftTab.c_str());
        send(shiftTab.c_str());
        CHECK_EQ(w1.focus(), &button2);
      }
      SUBCASE("A sequence is chained like in Qt") {
        w1.tabOrder({&button2, &drop, &area});
        // the first stays where it is, the others come right after it, one after the other
        CHECK_EQ(w1.tabChain(), std::vector<Widget *>{&label, &entry, &button1, &label2, &button2, &drop, &area});
        send(tab.c_str());
        for (Widget *expected: std::vector<Widget *>{&button1, &button2, &drop, &area, &entry}) {
          CHECK_EQ(w1.focus(), expected);
          send(tab.c_str());
        }
      }
      SUBCASE("It stays as it is") {
        w1.tabOrder(entry, entry);
        w1.tabOrder(button1, button1);
        w1.tabOrder({});
        w1.tabOrder({&entry});
        CHECK_EQ(w1.tabChain(), std::vector<Widget *>{&label, &entry, &button1, &area, &drop, &label2, &button2});
      }
      SUBCASE("Only widgets of this window") {
        auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{36, 0, 50, 6}, Text{"B"}, nullptr, &plain)};
        auto stranger{jwezel::ui::Button(w2, Rectangle{1, 1, 8, 2}, Text{"x"})};
        CHECK_THROWS_AS(w1.tabOrder(entry, stranger), std::invalid_argument);
        CHECK_THROWS_AS(w1.tabOrder(stranger, entry), std::invalid_argument);
        CHECK_THROWS_AS(w1.tabOrder({&entry, &stranger}), std::invalid_argument);
        CHECK_EQ(w1.tabChain().size(), 7);
      }
      SUBCASE("Widgets in boxes are where they were created") {
        auto box{jwezel::ui::VerticalBox(w1, Rectangle{22, 6, 33, 12})};
        auto in1{jwezel::ui::Button(box, Text{"in1"})};
        auto in2{jwezel::ui::Button(box, Text{"in2"})};
        CHECK_EQ(w1.tabChain().size(), 9);
        CHECK_EQ(w1.tabChain()[7], &in1);
        CHECK_EQ(w1.tabChain()[8], &in2);
        w1.focus(&button2);
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &in1);
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &in2);
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &entry);
      }
      SUBCASE("A widget that is destroyed is not in it anymore") {
        {
          auto temporary{jwezel::ui::Button(w1, Rectangle{22, 6, 30, 7}, Text{"tmp"})};
          CHECK_EQ(w1.tabChain().size(), 8);
          w1.focus(&temporary);
        }
        CHECK_EQ(w1.tabChain().size(), 7);
        CHECK_EQ(w1.focus(), nullptr);
        send(tab.c_str());
        CHECK_EQ(w1.focus(), &entry);
      }
    }
    SUBCASE("A widget that is covered completely is skipped") {
      Window cover{&term, Rectangle{1, 9, 20, 10}, '#'_C}; // over the drop down
      term.focus(&w1);
      REQUIRE(drop.fragments().empty());
      w1.focus(&area);
      send(tab.c_str());
      CHECK_EQ(w1.focus(), &button2);
      send(shiftTab.c_str());
      CHECK_EQ(w1.focus(), &area);
    }
    SUBCASE("A widget that is covered in part is not") {
      Window cover{&term, Rectangle{10, 9, 20, 10}, '#'_C};
      term.focus(&w1);
      w1.focus(&area);
      send(tab.c_str());
      CHECK_EQ(w1.focus(), &drop);
    }
    SUBCASE("Enter and Space press the focused button") {
      auto pressed{0};
      button1.onPress([&pressed](const jwezel::ui::Element &, const jwezel::Event &) {++pressed; return true;});
      send(tab.c_str());
      REQUIRE_EQ(w1.focus(), &button1);
      send(enter.c_str());
      CHECK_EQ(pressed, 1);
      send(" ");
      CHECK_EQ(pressed, 2);
      send("x");
      send(escape.c_str());
      CHECK_EQ(pressed, 2);
    }
  }

  SUBCASE("Actions") {
    auto send{
      [&](const char *input) {
        REQUIRE_NE(fputs(input, inputWrite), EOF);
        REQUIRE_FALSE(fflush(inputWrite));
        term.runEvent();
      }
    };
    auto click{
      [&](int x, int y) {
        send(std::format("\x1b[<0;{};{}M", x + 1, y + 1).c_str());
        send(std::format("\x1b[<0;{};{}m", x + 1, y + 1).c_str());
      }
    };
    using jwezel::Key;
    using jwezel::ui::Action;
    const std::vector<Text> fruit{Text{"Apple"}, Text{"Banana"}};
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 34, 12}, Text{"T"}, nullptr, &plain)};
    auto entry{jwezel::ui::LineEdit(w1, Rectangle{1, 1, 20, 2}, Text{"a"})};
    auto area{jwezel::ui::TextArea(w1, Rectangle{1, 3, 20, 5}, Text{"x"})};
    auto button{jwezel::ui::Button(w1, Rectangle{1, 6, 8, 7}, Text{"OK"})};
    auto drop{jwezel::ui::DropDown(w1, Rectangle{1, 8, 20, 9}, fruit)};
    std::string log;
    Action accept{[&log] {log += "accept;";}, {Key::Enter}};
    Action cancel{[&log] {log += "cancel;";}, {Key::Escape}};
    SUBCASE("A hidden action of the window: a default action and a cancel action") {
      w1.addAction(accept);
      w1.addAction(cancel);
      REQUIRE_EQ(w1.focus(), &entry);
      send("\r"); // an entry that does nothing with Enter lets the window have it
      CHECK_EQ(log, "accept;");
      send("\x1b");
      CHECK_EQ(log, "accept;cancel;");
      CHECK_EQ(entry.content().data[0].size(), 1); // nothing was typed
    }
    SUBCASE("From any widget that does not use the key") {
      w1.addAction(accept);
      w1.addAction(cancel);
      w1.focus(&button);
      send("\x1b");
      CHECK_EQ(log, "cancel;");
      w1.focus(&drop);
      send("\x1b[A"); // up in a closed drop down: nothing, and no action either
      CHECK_EQ(log, "cancel;");
    }
    SUBCASE("What the widget has made use of is not for the window") {
      w1.addAction(accept);
      w1.addAction(cancel);
      w1.focus(&drop);
      send("\r"); // opens the list
      REQUIRE(drop.isOpen());
      send("\x1b"); // closes it
      CHECK_FALSE(drop.isOpen());
      CHECK_EQ(log, "");
      send("\x1b"); // nothing is open anymore
      CHECK_EQ(log, "cancel;");
      w1.focus(&area);
      send("\r"); // a new line
      CHECK_EQ(area.content().data.size(), 2);
      CHECK_EQ(log, "cancel;");
    }
    SUBCASE("An accept handler of an entry that is for it") {
      w1.addAction(accept);
      auto handled{true};
      entry.onAccept([&log, &handled](const jwezel::ui::Element &, const jwezel::Event &) {log += "entry;"; return handled;});
      send("\r");
      CHECK_EQ(log, "entry;"); // used up
      handled = false;
      send("\r");
      CHECK_EQ(log, "entry;entry;accept;"); // it did not take care of it, so the window does as well
    }
    SUBCASE("Actions of the widget while it has the focus") {
      Action local{[&log] {log += "local;";}, {Key::F5}};
      Action everywhere{[&log] {log += "window;";}, {Key::F5, Key::F6}};
      entry.addAction(local);
      w1.addAction(everywhere);
      send("\x1b[15~"); // F5, in the entry
      CHECK_EQ(log, "local;"); // the widget comes first
      send("\x1b[17~"); // F6
      CHECK_EQ(log, "local;window;");
      w1.focus(&button);
      send("\x1b[15~");
      CHECK_EQ(log, "local;window;window;"); // not in the button
    }
    SUBCASE("A disabled action is passed over") {
      Action second{[&log] {log += "second;";}, {Key::Enter}};
      w1.addAction(accept);
      w1.addAction(second);
      accept.enabled(false);
      send("\r");
      CHECK_EQ(log, "second;");
      second.enabled(false);
      send("\r");
      CHECK_EQ(log, "second;");
    }
    SUBCASE("The window can have Tab") {
      Action own{[&log] {log += "tab;";}, {Key::Tab}};
      w1.addAction(own);
      send("\t");
      CHECK_EQ(log, "tab;");
      CHECK_EQ(w1.focus(), &entry); // it is not for the focus then
      own.enabled(false);
      send("\t");
      CHECK_EQ(w1.focus(), &area);
    }
    SUBCASE("A button triggers its actions when it is pressed") {
      Action press{[&log] {log += "press;";}};
      button.addAction(press);
      click(2, 6);
      CHECK_EQ(log, "press;");
      send("\r"); // the button has the focus now
      CHECK_EQ(log, "press;press;"); // once for the key, not twice
      send(" ");
      CHECK_EQ(log, "press;press;press;");
      press.enabled(false);
      send("\r");
      CHECK_EQ(log, "press;press;press;");
    }
    SUBCASE("A button and a key are the same action") {
      w1.addAction(accept);
      button.addAction(accept);
      click(2, 6);
      CHECK_EQ(log, "accept;");
      w1.focus(&entry);
      send("\r");
      CHECK_EQ(log, "accept;accept;");
    }
    SUBCASE("An action that is destroyed is gone") {
      {
        Action temporary{[&log] {log += "temporary;";}, {Key::F5}};
        w1.addAction(temporary);
        send("\x1b[15~");
        CHECK_EQ(log, "temporary;");
      }
      send("\x1b[15~"); // nothing there anymore, and nothing breaks
      CHECK_EQ(log, "temporary;");
    }
    SUBCASE("A window that is destroyed lets go of its actions") {
      {
        auto w2{jwezel::ui::Window(ui_, ' '_C, Rectangle{36, 0, 50, 6}, Text{"B"}, nullptr, &plain)};
        w2.addAction(accept);
        CHECK_EQ(accept.trigger(), true);
      }
      CHECK(accept.trigger()); // fine
      CHECK_EQ(log, "accept;accept;");
    }
    SUBCASE("The desktop is a window like any other") {
      // nothing has the focus on a desktop without widgets, and the actions are there all the same
      auto &desktop{ui_.desktop()};
      Action quit{[&log] {log += "quit;";}, {'\x03'}};
      desktop.addAction(quit);
      term.focus(&term.desktop());
      REQUIRE_EQ(term.focus(), &term.desktop());
      send("\x03");
      CHECK_EQ(log, "quit;");
    }
  }
  REQUIRE_FALSE(fclose(inputRead));
  REQUIRE_FALSE(fclose(inputWrite));
}

// NOLINTEND(misc-use-anonymous-namespace)
// NOLINTEND(readability-magic-numbers)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// NOLINTEND(cppcoreguidelines-avoid-c-arrays)
// NOLINTEND(modernize-avoid-c-arrays)
// NOLINTEND(hicpp-no-array-decay)
// NOLINTEND(hicpp-avoid-c-arrays)
// NOLINTEND(hicpp-no-assembler)
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
