#include <ui/text_buffer.hh>

#include <doctest/doctest.h>

#include <stdexcept>
#include <string>
#include <vector>

using
  jwezel::Vector,
  jwezel::ui::TextBuffer;

namespace doctest {
  template<> struct StringMaker<Vector> {
    static auto convert(const Vector &value) -> String {
      return std::string(value).c_str();
    }
  };
}

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

using Lines = std::vector<std::u32string>;

TEST_CASE("Text buffer: text") {
  SUBCASE("Empty") {
    const TextBuffer buffer;
    CHECK_EQ(buffer.lineCount(), 1); // there is always a line
    CHECK_EQ(buffer.lines(), Lines{U""});
    CHECK_EQ(buffer.text(), "");
    CHECK_EQ(buffer.end(), Vector{0, 0});
    CHECK_EQ(TextBuffer{""}.lines(), Lines{U""});
  }
  SUBCASE("Lines") {
    const TextBuffer buffer{"one\ntwo\n\nfour"};
    CHECK_EQ(buffer.lines(), Lines{U"one", U"two", U"", U"four"});
    CHECK_EQ(buffer.lineCount(), 4);
    CHECK_EQ(buffer.lineLength(0), 3);
    CHECK_EQ(buffer.lineLength(2), 0);
    CHECK_EQ(buffer.line(3), U"four");
    CHECK_EQ(buffer.text(), "one\ntwo\n\nfour"); // and back
    CHECK_EQ(buffer.end(), Vector{4, 3});
  }
  SUBCASE("The lines are not the same length, and there is nothing else to a line") {
    const TextBuffer buffer{"a\nlonger line\nb"};
    CHECK_EQ(buffer.lineLength(0), 1);
    CHECK_EQ(buffer.lineLength(1), 11);
    CHECK_EQ(buffer.lineLength(2), 1);
  }
  SUBCASE("A line feed at the end starts a line") {
    const TextBuffer buffer{"a\n"};
    CHECK_EQ(buffer.lines(), Lines{U"a", U""});
    CHECK_EQ(buffer.text(), "a\n");
    CHECK_EQ(TextBuffer{"\n"}.lines(), Lines{U"", U""});
    CHECK_EQ(TextBuffer{"\n\n"}.lineCount(), 3);
  }
  SUBCASE("Carriage returns before line feeds are not part of the text") {
    const TextBuffer buffer{"a\r\nb\r\n"};
    CHECK_EQ(buffer.lines(), Lines{U"a", U"b", U""});
    CHECK_EQ(buffer.text(), "a\nb\n");
  }
  SUBCASE("Characters are code points, not bytes") {
    const TextBuffer buffer{"Grüße €\n日本"};
    CHECK_EQ(buffer.lineLength(0), 7); // 9 bytes + 2 for the euro sign
    CHECK_EQ(buffer.lines()[0][2], U'ü');
    CHECK_EQ(buffer.lines()[0][6], U'€');
    CHECK_EQ(buffer.lineLength(1), 2);
    CHECK_EQ(buffer.text(), "Grüße €\n日本");
    CHECK_EQ(buffer.end(), Vector{2, 1});
  }
  SUBCASE("Bytes that are not UTF-8 do not get in") {
    const TextBuffer buffer{"a\xff" "b\xc3"}; // an impossible byte, a sequence that is not finished
    CHECK_EQ(buffer.lineCount(), 1);
    for (const auto character: buffer.lines()[0]) {
      CHECK_NE(character, 0xff);
    }
    CHECK_EQ(buffer.lines()[0].front(), U'a');
    CHECK_NE(buffer.text().find("\xef\xbf\xbd"), std::string::npos); // replaced by U+FFFD
  }
  SUBCASE("Replace") {
    TextBuffer buffer{"one\ntwo"};
    buffer.text("x");
    CHECK_EQ(buffer.lines(), Lines{U"x"});
    buffer.text("");
    CHECK_EQ(buffer.lines(), Lines{U""});
    buffer.text("a\nb\nc");
    CHECK_EQ(buffer.lineCount(), 3);
  }
  SUBCASE("It is a value") {
    TextBuffer buffer{"abc"};
    const auto copy{buffer};
    buffer.insert(Vector{3, 0}, U'd');
    CHECK_EQ(copy.text(), "abc");
    CHECK_EQ(buffer.text(), "abcd");
  }
}

TEST_CASE("Text buffer: positions") {
  const TextBuffer buffer{"ab\n\ncde"};
  CHECK(buffer.contains(Vector{0, 0}));
  CHECK(buffer.contains(Vector{2, 0})); // behind the last character
  CHECK_FALSE(buffer.contains(Vector{3, 0}));
  CHECK(buffer.contains(Vector{0, 1}));
  CHECK_FALSE(buffer.contains(Vector{1, 1}));
  CHECK(buffer.contains(Vector{3, 2}));
  CHECK_FALSE(buffer.contains(Vector{0, 3}));
  CHECK_FALSE(buffer.contains(Vector{-1, 0}));
  CHECK_FALSE(buffer.contains(Vector{0, -1}));
  CHECK_THROWS_AS((void)buffer.line(3), std::out_of_range);
  CHECK_THROWS_AS((void)buffer.line(-1), std::out_of_range);
}

TEST_CASE("Text buffer: editing") {
  TextBuffer buffer{"abc\ndef"};
  SUBCASE("Insert") {
    buffer.insert(Vector{0, 0}, U'x');
    buffer.insert(Vector{2, 0}, U'y');
    buffer.insert(Vector{5, 0}, U'z'); // at the end
    CHECK_EQ(buffer.text(), "xaybcz\ndef");
    buffer.insert(Vector{3, 1}, U'!');
    CHECK_EQ(buffer.line(1), U"def!");
    buffer.insert(Vector{0, 1}, U'€');
    CHECK_EQ(buffer.line(1), U"€def!");
  }
  SUBCASE("Insert into a line that is empty") {
    buffer.text("");
    buffer.insert(Vector{0, 0}, U'a');
    CHECK_EQ(buffer.text(), "a");
  }
  SUBCASE("Split") {
    buffer.split(Vector{1, 0});
    CHECK_EQ(buffer.lines(), Lines{U"a", U"bc", U"def"});
    buffer.split(Vector{0, 0}); // an empty line before
    CHECK_EQ(buffer.lines(), Lines{U"", U"a", U"bc", U"def"});
    buffer.split(Vector{3, 3}); // at the end: an empty line behind
    CHECK_EQ(buffer.lines(), Lines{U"", U"a", U"bc", U"def", U""});
  }
  SUBCASE("Erase") {
    buffer.erase(Vector{1, 0});
    CHECK_EQ(buffer.line(0), U"ac");
    buffer.erase(Vector{0, 0});
    CHECK_EQ(buffer.line(0), U"c");
    buffer.erase(Vector{1, 0}); // at the end of a line, the next one is joined
    CHECK_EQ(buffer.lines(), Lines{U"cdef"});
    buffer.erase(Vector{4, 0}); // at the end of everything there is nothing
    CHECK_EQ(buffer.lines(), Lines{U"cdef"});
  }
  SUBCASE("Erase joins an empty line") {
    buffer.text("a\n\nb");
    buffer.erase(Vector{1, 0});
    CHECK_EQ(buffer.lines(), Lines{U"a", U"b"});
    buffer.text("a\n\nb");
    buffer.erase(Vector{0, 1});
    CHECK_EQ(buffer.lines(), Lines{U"a", U"b"});
  }
  SUBCASE("Erase before") {
    CHECK_EQ(buffer.eraseBefore(Vector{2, 0}), Vector{1, 0});
    CHECK_EQ(buffer.line(0), U"ac");
    CHECK_EQ(buffer.eraseBefore(Vector{2, 0}), Vector{1, 0});
    CHECK_EQ(buffer.line(0), U"a");
    CHECK_EQ(buffer.eraseBefore(Vector{0, 0}), Vector{0, 0}); // nothing before the beginning
    CHECK_EQ(buffer.text(), "a\ndef");
  }
  SUBCASE("Erase before the beginning of a line joins it to the previous one") {
    CHECK_EQ(buffer.eraseBefore(Vector{0, 1}), Vector{3, 0}); // where the text of the first line ends
    CHECK_EQ(buffer.lines(), Lines{U"abcdef"});
    buffer.text("a\n\nb");
    CHECK_EQ(buffer.eraseBefore(Vector{0, 1}), Vector{1, 0});
    CHECK_EQ(buffer.lines(), Lines{U"a", U"b"});
    CHECK_EQ(buffer.eraseBefore(Vector{0, 1}), Vector{1, 0});
    CHECK_EQ(buffer.lines(), Lines{U"ab"});
  }
  SUBCASE("Positions that are not in the text are an error, and change nothing") {
    for (const auto &position: {Vector{4, 0}, Vector{0, 2}, Vector{-1, 0}, Vector{0, -1}, Vector{9, 9}}) {
      INFO("Position: ", std::string(position));
      CHECK_THROWS_AS(buffer.insert(position, U'x'), std::out_of_range);
      CHECK_THROWS_AS(buffer.split(position), std::out_of_range);
      CHECK_THROWS_AS(buffer.erase(position), std::out_of_range);
      CHECK_THROWS_AS((void)buffer.eraseBefore(position), std::out_of_range);
      CHECK_EQ(buffer.text(), "abc\ndef");
    }
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(readability-magic-numbers)
