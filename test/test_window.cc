#include "geometry.hh"
#include "term.hh"
#include "text.hh"
#include "window.hh"

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

TEST_CASE("Window") {
  auto *output{tmpfile()};
  auto *input{tmpfile()};
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[10;20R", input);
  (void)fseek(input, 0, SEEK_SET);
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, output->_fileno, input->_fileno);
  SUBCASE("Window") {
    auto w1 = Window(&term, Rectangle{0, 0, 10, 6}, Char{' ', jwezel::RgbWhite, jwezel::RgbBlue1});
    auto tt = Text("          \n          \n          \n          \n          \n          ", jwezel::RgbWhite, jwezel::RgbBlue1);
    CHECK_EQ(term.display().text(), tt);
    SUBCASE("Box") {
      w1.box();
      tt.withBox();
      CHECK_EQ(term.display().text(), tt);
      SUBCASE("Title") {
        w1.write(Vector{1, 0}, Text{"Title", jwezel::RgbRed, jwezel::RgbGray7, clear, mix});
        tt.patch(Text{"Title", jwezel::RgbRed, jwezel::RgbGray7, clear, mix}, Vector{1, 0});
        CHECK_EQ(term.display().text(), tt);
        SUBCASE("Line") {
          w1.line(Line{Vector{0, 2}});
          tt.withLine(Line{Vector{0, 2}});
          CHECK_EQ(term.display().text(), tt);
          SUBCASE("Fill") {
            w1.fill('.'_C, Rectangle{2, 3, 4, 4});
            tt.fill('.'_C, Rectangle{2, 3, 4, 4});
            CHECK_EQ(term.display().text(), tt);
          }
        }
      }
    }
  }
}

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
