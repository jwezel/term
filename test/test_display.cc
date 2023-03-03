#include <cstdio>
#include <doctest/doctest.h>
#include <unistd.h>

#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "string.hh"
#include "text.hh"

using
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
    SUBCASE("Updates") {
      disp.resize(Vector{10, 4});
      disp.update(Vector{0, 0}, Text("::::::::::\n::::::::::\n::::::::::\n::::::::::"));
      disp.update(Vector{1, 1}, Text("++++++++\n++++++++"));
      CHECK_EQ(disp.text_.repr(), Text("::::::::::\n:++++++++:\n:++++++++:\n::::::::::").repr());
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
