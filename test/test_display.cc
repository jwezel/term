#include <term/display.hh>
#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <util/string.hh>
#include <term/text.hh>

#include <cstdio>
#include <cstring>
#include <regex>
#include <doctest/doctest.h>
#include <unistd.h>

using
  jwezel::Attributes,
  jwezel::blink,
  jwezel::bold,
  jwezel::Dim,
  jwezel::Display,
  jwezel::Keyboard,
  jwezel::repr,
  jwezel::reverse,
  jwezel::RgbBlue,
  jwezel::string,
  jwezel::Text,
  jwezel::toDim,
  jwezel::underline,
  jwezel::Vector;

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

static const auto BufferSize{2048};

TEST_CASE("Display") {
  SUBCASE("Simulated") {
    auto *output{tmpfile()};
    auto *input{tmpfile()};
    // Simulate terminal replies for cursor position
    CHECK_EQ(fputs("\x1b[1;1R", input), 1);
    CHECK_EQ(fputs("\x1b[1;1R", input), 1);
    CHECK_EQ(fputs("\x1b[20;10R", input), 1);
    CHECK_EQ(fseek(input, 0, SEEK_SET), 0);
    Keyboard kb(input->_fileno);
    Display disp{kb, output->_fileno};
    CHECK_EQ(fflush(output), 0);
    ftruncate(output->_fileno, 0); // Constructor requests cursor pos (writes to display)
    CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
    SUBCASE("write") {
      char buffer[BufferSize];
      disp.write("something");
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(repr(buffer), repr("something"));
    }
    SUBCASE("Turn cursor off") {
      char buffer[BufferSize];
      disp.cursor(false);
      disp.cursor(true);
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), string("\x1b[?25l\x1b[?25h"));
    }
    SUBCASE("Foreground") {
      char buffer[BufferSize];
      disp.foreground(RgbBlue);
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), string("\x1b[38;2;0;0;255m"));
    }
    SUBCASE("Background") {
      char buffer[BufferSize];
      disp.background(RgbBlue);
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), string("\x1b[48;2;0;0;255m"));
    }
    SUBCASE("Transparent colors are the terminal default") {
      char buffer[BufferSize];
      memset(buffer, 0, sizeof buffer);
      disp.foreground(jwezel::RgbTransparent);
      disp.foreground(jwezel::RgbNone); // nothing more to write
      disp.background(jwezel::RgbTransparent); // the background is none already: nothing to write
      disp.background(RgbBlue);
      disp.background(jwezel::RgbTransparent);
      disp.background(jwezel::RgbTransparent);
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), string("\x1b[39m\x1b[48;2;0;0;255m\x1b[49m"));
    }
    SUBCASE("Attributes") {
      char buffer[BufferSize];
      memset(buffer, 0, sizeof buffer);
      disp.write("@");
      disp.attributes(bold | underline | reverse | blink);
      disp.write("@");
      disp.attributes(0);
      disp.write("@");
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), "@\x1b[1;4;7;5m@\x1b[22;24;27;25m@");
    }
    SUBCASE("Attributes: any change is one well-formed sequence") {
      // From every set of attributes to every other: one sequence that starts with the escape, or nothing
      const std::regex wellFormed{"(\x1b\\[[0-9]+(;[0-9]+)*m)?"};
      const auto fd{output->_fileno};
      for (Attributes from = 0; from < 16; ++from) {
        for (Attributes to = 0; to < 16; ++to) {
          disp.attributes(from);
          REQUIRE_EQ(ftruncate(fd, 0), 0);
          REQUIRE_EQ(lseek(fd, 0, SEEK_SET), 0);
          disp.attributes(to);
          char buffer[BufferSize];
          memset(buffer, 0, sizeof buffer);
          REQUIRE_EQ(lseek(fd, 0, SEEK_SET), 0);
          const auto length{read(fd, buffer, sizeof buffer - 1)};
          INFO("from ", int(from), " to ", int(to), ": ", repr(buffer));
          CHECK(std::regex_match(string(buffer, static_cast<size_t>(length)), wellFormed));
          CHECK_EQ(length == 0, from == to);
        }
      }
    }
    SUBCASE("Attributes: reverse only") {
      char buffer[BufferSize];
      memset(buffer, 0, sizeof buffer);
      disp.attributes(reverse); // used to be written without the escape: ";7m"
      disp.attributes(0);
      disp.attributes(underline | reverse);
      disp.attributes(bold | underline);
      CHECK_EQ(fseek(output, 0, SEEK_SET), 0);
      CHECK_NE(fgets(buffer, sizeof buffer, output), nullptr);
      CHECK_EQ(string(buffer), "\x1b[7m\x1b[27m\x1b[4;7m\x1b[1;27m");
    }
    SUBCASE("Updates") {
      disp.resize(Vector{10, 4});
      disp.update(Vector{0, 0}, Text("::::::::::\n::::::::::\n::::::::::\n::::::::::"));
      disp.update(Vector{1, 1}, Text("++++++++\n++++++++"));
      CHECK_EQ(disp.text().repr(), Text("::::::::::\n:++++++++:\n:++++++++:\n::::::::::").repr());
    }
    SUBCASE("Resize") {
      disp.resize(Vector{10, 6});
      CHECK_EQ(disp.size(), Vector{10, 6});
    }
    // fclose(input);
    // fclose(output);
    kb.reset();
  }
}
TEST_CASE("Optional tests" * doctest::skip(true)) {
  SUBCASE("Real") {
    Keyboard kb(0);
    kb.raw();
    Display disp{kb};
    auto pos{disp.cursor()};
    CHECK_GE(pos.x(), 0);
    CHECK_LE(pos.x(), 300);
    CHECK_GE(pos.y(), 0);
    CHECK_LE(pos.y(), 100);
    SUBCASE("Set cursor") {
      disp.cursor(toDim(pos.x() + 10), pos.y());
      auto cp{disp.cursor()};
      CHECK_EQ(cp, Vector{Dim(pos.x() + 10), pos.y()});
    }
    kb.reset();
  }
}

// NOLINTBEGIN(hicpp-avoid-c-arrays)
// NOLINTBEGIN(misc-use-anonymous-namespace)
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
