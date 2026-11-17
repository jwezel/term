#include "fake_terminal.hh"

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
  jwezel::MouseButtonEvent,
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
TEST_CASE("Display on the main screen scrolls the terminal to make room") {
  // The terminal is 10 wide and 20 high, and the cursor is at column 3 of line 18
  FakeTerminal terminal{"\x1b[18;3R\x1b[18;3R\x1b[20;10R"};
  Keyboard keyboard{terminal.input()};
  Display display{keyboard, terminal.output()};
  CHECK_EQ(display.screen(), Display::Screen::main);
  CHECK_EQ(display.position(), Vector{2, 17});
  CHECK_EQ(display.maxSize(), Vector{8, 20}); // as wide as there is room, as high as the terminal: it scrolls
  terminal.clear();
  SUBCASE("Nothing is scrolled while there is room") {
    display.resize(Vector{8, 3}); // exactly the three lines that are left
    CHECK_EQ(terminal.written(), "");
    CHECK_EQ(display.position(), Vector{2, 17});
    CHECK_EQ(display.size(), Vector{8, 3});
    display.resize(Vector{4, 2}); // smaller
    CHECK_EQ(terminal.written(), "");
  }
  SUBCASE("It scrolls by what is missing") {
    display.resize(Vector{8, 6});
    // go to the last line, where line feeds scroll the terminal
    CHECK_EQ(terminal.written(), "\x1b[20;3H\n\n\n");
    CHECK_EQ(display.position(), Vector{2, 14}); // it is up by three lines
    CHECK_EQ(display.size(), Vector{8, 6});
    terminal.clear();
    display.update(Vector{0, 0}, Text{"a"});
    CHECK_NE(terminal.written().find("\x1b[15;3H"), std::string::npos); // the first cell is on the line 14 now
    SUBCASE("Mouse reports are relative to the display where it is now") {
      terminal.feed("\x1b[<0;5;16M"); // column 5, line 16 of the terminal
      auto event{keyboard.event()};
      REQUIRE_EQ(event.type(), MouseButtonEvent::type_);
      CHECK_EQ(dynamic_cast<MouseButtonEvent *>(event())->position(), Vector{2, 1});
    }
    SUBCASE("And again") {
      terminal.clear();
      display.resize(Vector{8, 8}); // two more lines
      CHECK_EQ(display.position(), Vector{2, 12});
      CHECK_NE(terminal.written().find("\n\n"), std::string::npos);
    }
  }
  SUBCASE("Not higher than the terminal") {
    display.resize(Vector{8, 30});
    CHECK_EQ(display.size(), Vector{8, 20});
    CHECK_EQ(display.position(), Vector{2, 0}); // all the way up
    CHECK_EQ(terminal.written(), "\x1b[20;3H" + std::string(17, '\n'));
  }
  SUBCASE("Not wider than the room to the right") {
    display.resize(Vector{20, 2});
    CHECK_EQ(display.size(), Vector{8, 2});
  }
}

TEST_CASE("Display on the main screen: start size and position") {
  SUBCASE("A start size that does not fit scrolls when the display is created") {
    FakeTerminal terminal{"\x1b[18;3R\x1b[18;3R\x1b[20;10R"};
    Keyboard keyboard{terminal.input()};
    Display display{keyboard, terminal.output(), jwezel::VectorMin, Vector{8, 6}};
    CHECK_EQ(display.position(), Vector{2, 14});
    CHECK_EQ(display.size(), Vector{8, 6});
    CHECK_NE(terminal.written().find("\x1b[20;3H\n\n\n"), std::string::npos);
  }
  SUBCASE("A position that is given is where the display is, and it scrolls from there") {
    FakeTerminal terminal{"\x1b[3;3R\x1b[3;3R\x1b[20;10R"};
    Keyboard keyboard{terminal.input()};
    Display display{keyboard, terminal.output(), Vector{1, 16}};
    CHECK_EQ(display.position(), Vector{1, 16});
    CHECK_EQ(display.maxSize(), Vector{9, 20});
    display.resize(Vector{9, 6});
    CHECK_EQ(display.position(), Vector{1, 14}); // 16 + 6 = 22: two lines too many
  }
  SUBCASE("The cursor is not moved if the display has room") {
    FakeTerminal terminal{"\x1b[3;1R\x1b[3;1R\x1b[20;10R"};
    Keyboard keyboard{terminal.input()};
    Display display{keyboard, terminal.output()};
    terminal.clear();
    display.resize(Vector{10, 10});
    CHECK_EQ(terminal.written(), "");
    CHECK_EQ(display.position(), Vector{0, 2});
  }
}

TEST_CASE("Display on the alternate screen") {
  // The terminal is 10 wide and 20 high; the cursor is wherever the shell left it, which is of no interest
  const std::string replies{"\x1b[5;5R\x1b[5;5R\x1b[20;10R"};
  SUBCASE("It is fixed") {
    FakeTerminal terminal{replies};
    {
      Keyboard keyboard{terminal.input()};
      Display display{keyboard, terminal.output(), jwezel::VectorMin, jwezel::VectorMin, jwezel::VectorMax, Display::Screen::alternate};
      CHECK_EQ(display.screen(), Display::Screen::alternate);
      CHECK_EQ(terminal.written().substr(0, 15), "\x1b[?1049h\x1b[2J\x1b[H"); // the first thing that is written
      CHECK_EQ(display.position(), Vector{0, 0}); // not where the cursor was
      CHECK_EQ(display.maxSize(), Vector{10, 20}); // the whole terminal
      terminal.clear();
      display.resize(Vector{10, 20});
      CHECK_EQ(display.size(), Vector{10, 20});
      CHECK_EQ(display.position(), Vector{0, 0});
      CHECK_EQ(terminal.written().find('\n'), std::string::npos); // there is nothing to scroll
      display.resize(Vector{30, 30});
      CHECK_EQ(display.size(), Vector{10, 20});
      terminal.clear();
    }
    // The screen is given up again, which brings back the cursor of the main screen, so the display does not move it
    CHECK_EQ(terminal.written().substr(terminal.written().size() - 8), "\x1b[?1049l");
    CHECK_EQ(terminal.written().find('\n'), std::string::npos);
    CHECK_NE(terminal.written().find("\x1b[?1003l"), std::string::npos); // mouse reporting is off
  }
  SUBCASE("The size of the display can be limited") {
    FakeTerminal terminal{replies};
    Keyboard keyboard{terminal.input()};
    Display display{keyboard, terminal.output(), jwezel::VectorMin, Vector{4, 4}, Vector{6, 8}, Display::Screen::alternate};
    CHECK_EQ(display.maxSize(), Vector{6, 8});
    CHECK_EQ(display.size(), Vector{4, 4});
  }
  SUBCASE("The top left corner is the position it has anyway") {
    FakeTerminal terminal{replies};
    Keyboard keyboard{terminal.input()};
    Display display{keyboard, terminal.output(), Vector{0, 0}, jwezel::VectorMin, jwezel::VectorMax, Display::Screen::alternate};
    CHECK_EQ(display.position(), Vector{0, 0});
  }
  SUBCASE("Any other position is an error, which is found before anything is written") {
    FakeTerminal terminal{replies};
    Keyboard keyboard{terminal.input()};
    CHECK_THROWS_AS(
      Display(keyboard, terminal.output(), Vector{3, 3}, jwezel::VectorMin, jwezel::VectorMax, Display::Screen::alternate),
      std::invalid_argument
    );
    CHECK_EQ(terminal.written(), ""); // the screen was not switched, so nothing has to be switched back
  }
  SUBCASE("The main screen is not switched") {
    FakeTerminal terminal{"\x1b[1;1R\x1b[1;1R\x1b[20;10R"};
    {
      Keyboard keyboard{terminal.input()};
      Display display{keyboard, terminal.output()};
    }
    CHECK_EQ(terminal.written().find("1049"), std::string::npos);
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
