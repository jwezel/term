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
  auto *inputWrite{fopen(tempFilename, "ae")};
  auto *inputRead{fopen(tempFilename, "re")};
  MESSAGE(std::string(tempFilename));
  MESSAGE(std::string(outputFilename));
  REQUIRE_NE(fputs("\x1b[1;1R", inputWrite), EOF);
  REQUIRE_NE(fputs("\x1b[1;1R", inputWrite), EOF);
  REQUIRE_NE(fputs("\x1b[10;20R", inputWrite), EOF);
  REQUIRE_FALSE(fflush(inputWrite));
  CAPTURE(ftell(inputWrite));
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, output->_fileno, inputRead->_fileno);
  jwezel::ui::Ui ui_(&term);

  SUBCASE("Window") {
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 10, 6}, Text{"Title"})};
    auto test{Text{' '_C, Vector{10, 6}}};
    test.box();
    test.patch(Text{"Title"}, Vector{1, 0});
    CHECK_EQ(w1.text(), test);
    CHECK_EQ(term.display().text(), test);
    SUBCASE("Widget") {
      auto w2{jwezel::ui::Widget(Rectangle{1, 1, 8, 2}, ' '_C, &w1)};
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
    CHECK_EQ(term.display().text(), test);
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
