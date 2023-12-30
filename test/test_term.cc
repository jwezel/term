#include <cstdio>
#include <doctest/doctest.h>

#include "geometry.hh"
#include "term.hh"
#include "window.hh"
#include "text.hh"

using
  jwezel::Vector,
  jwezel::VectorMin,
  jwezel::VectorMax,
  jwezel::Rectangle,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::string,
  jwezel::Window,
  jwezel::operator""_C;

namespace doctest {
    template<> struct StringMaker<Vector> {
        static auto convert(const Vector& value) -> String {
            return string(value).c_str();
        }
    };
} // namespace doctest

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

TEST_CASE("Terminal") {
  auto *output{tmpfile()};
  auto *input{tmpfile()};
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[10;20R", input);
  (void)fseek(input, 0, SEEK_SET);
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, output->_fileno, input->_fileno);
  SUBCASE("Window") {
    Window w1(term, Rectangle{0, 0, 10, 4}, '1'_C);
    CHECK_EQ(string(term.display().size()), string(Vector{10, 4}));
    CHECK_EQ(string(term.desktop().text().size()), string(Vector{10, 4}));
    CHECK_EQ(term.display().text().repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr());
    SUBCASE("Delete window 2") {
      SUBCASE("Another window") {
        Window w2{term, Rectangle{2, 2, 8, 6}, '2'_C};
        CHECK_EQ(string(term.display().size()), string(Vector{10, 6}));
        CHECK_EQ(string(term.desktop().text().size()), string(Vector{10, 6}));
        CHECK_EQ(term.display().text().repr(), Text("1111111111\n1111111111\n1122222211\n1122222211\n..222222..\n..222222..").repr());
          SUBCASE("Move window 2") {
            w2.move(Rectangle{2, 2, 12, 7});
            CHECK_EQ(
              term.display().text().repr(),
              Text("1111111111..\n1111111111..\n112222222222\n112222222222\n..2222222222\n..2222222222\n..2222222222").repr()
            );
          }
      }
      CHECK_EQ(term.display().text().repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr()); // display contracted
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
