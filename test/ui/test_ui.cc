#include "ui/button.hh"
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
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 20, 12}, Text{"Title"})};
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
      test2.patch(Text{"[ OK ]"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
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
      SUBCASE("Press with mouse") {
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
        CHECK(called);
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
    auto w1{jwezel::ui::Window(ui_, ' '_C, placement, Text{"Title"})};
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
      CHECK(pressed);
      CHECK_FALSE(term.display().caret()); // a button has no caret
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
