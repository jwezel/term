#include "fake_terminal.hh"

#include <term/geometry.hh>
#include <term/term.hh>
#include <term/text.hh>

#include <cstdio>
#include <string>
#include <doctest/doctest.h>

using
  jwezel::Display,
  jwezel::MouseButtonEvent,
  jwezel::operator""_C,
  jwezel::Rectangle,
  jwezel::string,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin,
  jwezel::Window;

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
    Window w1(&term, Rectangle{0, 0, 10, 4}, '1'_C);
    CHECK_EQ(string(term.display().size()), string(Vector{10, 4}));
    CHECK_EQ(string(term.desktop().text().size()), string(Vector{10, 4}));
    CHECK_EQ(term.display().text().repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr());
    SUBCASE("Delete window 2") {
      SUBCASE("Another window") {
        Window w2{&term, Rectangle{2, 2, 8, 6}, '2'_C};
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

TEST_CASE("Terminal makes room for its display by scrolling") {
  // The terminal is 20 wide and 10 high, and the cursor is in line 8, so there are three lines left
  const std::string replies{"\x1b[8;1R\x1b[8;1R\x1b[10;20R"};
  FakeTerminal fake{replies};
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, fake.output(), fake.input());
  CHECK_EQ(term.display().position(), Vector{0, 7});
  CHECK_EQ(term.display().maxSize(), Vector{20, 10}); // the whole terminal can be used, not just what is below the cursor
  fake.clear();
  SUBCASE("Expanding for a window that does not fit scrolls the terminal") {
    Window w1(&term, Rectangle{0, 0, 10, 4}, '1'_C);
    CHECK_EQ(term.display().size(), Vector{10, 4});
    CHECK_EQ(term.display().position(), Vector{0, 6}); // 7 + 4 = 11: one line too many
    CHECK_EQ(fake.written().substr(0, 8), "\x1b[10;2H\n"); // the last line, in the column where the cursor is, and a line feed
    CHECK_EQ(term.display().text().repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr());
    CHECK_EQ(term.desktop().text().size(), Vector{10, 4});
    SUBCASE("and so does another one") {
      Window w2{&term, Rectangle{2, 2, 8, 9}, '2'_C};
      CHECK_EQ(term.display().size(), Vector{10, 9});
      CHECK_EQ(term.display().position(), Vector{0, 1}); // 6 + 9 = 15: five more lines
      CHECK_EQ(
        term.display().text().repr(),
        Text("1111111111\n1111111111\n1122222211\n1122222211\n..222222..\n..222222..\n..222222..\n..222222..\n..222222..").repr()
      );
      SUBCASE("Mouse reports are relative to where the display is") {
        fake.feed("\x1b[<0;4;4M"); // column 4, line 4 of the terminal: column 3, line 2 of the display
        auto event{term.event()};
        REQUIRE_EQ(event.type(), MouseButtonEvent::type_);
        CHECK_EQ(dynamic_cast<MouseButtonEvent *>(event())->position(), Vector{3, 2});
      }
      SUBCASE("The display stays where it is when it contracts") {
        fake.clear();
        w2.move(Rectangle{2, 2, 8, 5});
        CHECK_EQ(term.display().position(), Vector{0, 1});
      }
    }
  }
  SUBCASE("A window that fits does not scroll anything") {
    Window w1(&term, Rectangle{0, 0, 10, 3}, '1'_C);
    CHECK_EQ(term.display().position(), Vector{0, 7});
    CHECK_EQ(fake.written().find('\n'), std::string::npos);
  }
  SUBCASE("A window is not larger than the terminal") {
    Window w1(&term, Rectangle{0, 0, 30, 15}, '1'_C);
    CHECK_EQ(term.display().size(), Vector{20, 10});
    CHECK_EQ(term.display().position(), Vector{0, 0});
  }
}

TEST_CASE("Terminal on the alternate screen") {
  FakeTerminal fake{"\x1b[5;5R\x1b[5;5R\x1b[10;20R"}; // the cursor is somewhere, the terminal is 20 x 10
  {
    Terminal term('.'_C, VectorMin, VectorMin, VectorMax, fake.output(), fake.input(), true, true, Display::Screen::alternate);
    CHECK_EQ(term.display().screen(), Display::Screen::alternate);
    CHECK_EQ(term.display().position(), Vector{0, 0});
    fake.clear();
    Window w1(&term, Rectangle{0, 0, 20, 10}, '1'_C);
    CHECK_EQ(term.display().size(), Vector{20, 10});
    CHECK_EQ(term.display().position(), Vector{0, 0}); // fixed: nothing to scroll
    CHECK_EQ(fake.written().find('\n'), std::string::npos);
    CHECK_THROWS_AS(
      Terminal(
        '.'_C, Vector{2, 2}, VectorMin, VectorMax, fake.output(), fake.input(), true, true, Display::Screen::alternate
      ),
      std::invalid_argument
    );
    fake.feed("\x1b[1;1R\x1b[1;1R\x1b[10;20R"); // for the one that could not be created
  }
  CHECK_EQ(fake.written().substr(fake.written().size() - 8), "\x1b[?1049l");
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
