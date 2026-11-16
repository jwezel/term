#include "ui/button.hh"
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
      [&](int line) { // the content of a row; the highlight (reverse, and the mix it leaves behind) is checked separately
        auto result{term.display().text()[Rectangle{1, jwezel::toDim(line), 19, jwezel::toDim(line + 1)}]};
        for (auto &cell: result.data.at(0)) {
          cell.attributes.attr = static_cast<jwezel::Attributes>(cell.attributes.attr & ~jwezel::reverse);
          cell.attributes.mix = jwezel::AttributeMode::default_;
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
