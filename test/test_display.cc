#include <cstdio>
#include <doctest/doctest.h>
#include <unistd.h>

#include "display.hh"
#include "fmt/core.h"
#include "fmt/format.h"
#include "geometry.hh"
#include "keyboard.hh"
#include "text.hh"
#include "string.hh"

TEST_CASE("Display") {
  SUBCASE("Simulated") {
    auto output{tmpfile()};
    auto input{tmpfile()};
    // Simulate terminal replies for cursor position
    fputs("\x1b[1;1R", input);
    fputs("\x1b[1;1R", input);
    fputs("\x1b[20;10R", input);
    fseek(input, 0, SEEK_SET);
    Keyboard k(input->_fileno);
    Display d{k, output->_fileno};
    fflush(output);
    ftruncate(output->_fileno, 0); // Constructor requests cursor pos (writes to display)
    fseek(output, 0, SEEK_SET);
    SUBCASE("write") {
      char buffer[2048];
      d.write("something");
      fseek(output, 0, SEEK_SET);
      fgets(buffer, sizeof buffer, output);
      CHECK_EQ(repr(buffer), repr("something"));
    }
    SUBCASE("Turn cursor off") {
      char buffer[2048];
      d.cursor(false);
      d.cursor(true);
      fseek(output, 0, SEEK_SET);
      fgets(buffer, sizeof buffer, output);
      CHECK_EQ(string(buffer), string("\x1b[?25l\x1b[?25h"));
    }
    SUBCASE("Foreground") {
      char buffer[2048];
      d.foreground(RgbBlue);
      fseek(output, 0, SEEK_SET);
      fgets(buffer, sizeof buffer, output);
      CHECK_EQ(string(buffer), string("\x1b[38;2;0;0;255m"));
    }
    SUBCASE("Background") {
      char buffer[2048];
      d.background(RgbBlue);
      fseek(output, 0, SEEK_SET);
      fgets(buffer, sizeof buffer, output);
      CHECK_EQ(string(buffer), string("\x1b[48;2;0;0;255m"));
    }
    SUBCASE("Attributes") {
      char buffer[2048];
      memset(buffer, 0, sizeof buffer);
      d.attributes(bold | underline | jwezel::reverse | blink);
      d.attributes(0);
      fseek(output, 0, SEEK_SET);
      fgets(buffer, sizeof buffer, output);
      CHECK_EQ(string(buffer), string("\x1b[1;4;7;5m\x1b[21;24;27;25m"));
    }
    SUBCASE("Updates") {
      d.resize({10, 4});
      d.update({0, 0}, Text("::::::::::\n::::::::::\n::::::::::\n::::::::::"));
      d.update({1, 1}, Text("++++++++\n++++++++"));
      CHECK_EQ(d.text_.repr(), Text("::::::::::\n:++++++++:\n:++++++++:\n::::::::::").repr());
    }
    SUBCASE("Resize") {
      d.resize(Vector{10, 6});
      CHECK_EQ(d.size(), Vector{10, 6});
    }
    // fclose(input);
    // fclose(output);
    k.reset();
  }
}
TEST_CASE("Optional tests" * doctest::skip(true)) {
  SUBCASE("Real") {
    Keyboard k(0);
    k.raw();
    Display d{k};
    auto pos{d.cursor()};
    CHECK_GE(pos.x, 0);
    CHECK_LE(pos.x, 300);
    CHECK_GE(pos.y, 0);
    CHECK_LE(pos.y, 100);
    SUBCASE("Set cursor") {
      d.cursor(pos.x + 10, pos.y);
      auto c{d.cursor()};
      CHECK_EQ(c, Vector{Dim(pos.x + 10), pos.y});
    }
    k.reset();
  }
}
