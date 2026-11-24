#include <term/geometry.hh>
#include <term/text.hh>

#include <doctest/doctest.h>

using
  jwezel::u1,
  jwezel::AttributeMode,
  jwezel::Attributes,
  jwezel::bold,
  jwezel::Box,
  jwezel::Char,
  jwezel::CharAttributes,
  jwezel::DimHigh,
  jwezel::Hsv,
  jwezel::Line,
  jwezel::AttributeMode,
  jwezel::Rectangle,
  jwezel::Rgb,
  jwezel::RgbBlue,
  jwezel::RgbCyan1,
  jwezel::RgbGray1,
  jwezel::RgbGray2,
  jwezel::RgbGreen,
  jwezel::RgbNone,
  jwezel::RgbYellow,
  jwezel::string,
  jwezel::Text,
  jwezel::underline,
  jwezel::Vector;

namespace doctest {
    template<> struct StringMaker<Text> {
        static auto convert(const Text& value) -> String {
            return value.repr().c_str();
        }
    };

    template<> struct StringMaker<Rgb> {
        static auto convert(const Rgb& value) -> String {
            return string(value).c_str();
        }
    };

    template<> struct StringMaker<Hsv> {
        static auto convert(const Hsv& value) -> String {
            return string(value).c_str();
        }
    };

    template<> struct StringMaker<CharAttributes> {
        static auto convert(const CharAttributes& value) -> String {
            return string(value).c_str();
        }
    };

    template<> struct StringMaker<Char> {
        static auto convert(const Char& value) -> String {
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

TEST_CASE("Color") {
  SUBCASE("Rgb") {
    Rgb rgb{1., 0., 0.};
    SUBCASE("Mix") {
      Rgb rgb2{rgb + Rgb{0., 1., 0.}};
      CHECK_EQ(rgb2, Rgb{0.5, 0.5, 0.});
    }
    SUBCASE("To HSV") {
      Hsv hsv{rgb};
      CHECK_EQ(hsv, Hsv{0., 1., 1.});
    }
  }
  SUBCASE("Hsv") {
    SUBCASE("Red") {
      Hsv hsv{0., 1., 1.};
      SUBCASE("To RGB") {
        Rgb rgb{hsv};
        CHECK_EQ(rgb, Rgb{1., 0., 0.});
      }
    }
    SUBCASE("Green") {
      Hsv hsv{120., 1., 1.};
      SUBCASE("To RGB") {
        Rgb rgb{hsv};
        CHECK_EQ(rgb, Rgb{0., 1., 0.});
      }
    }
    SUBCASE("Blue") {
      Hsv hsv{240., 1., 1.};
      SUBCASE("To RGB") {
        Rgb rgb{hsv};
        CHECK_EQ(rgb, Rgb{0., 0., 1.});
      }
    }
    SUBCASE("Yellow") {
      Hsv hsv{60., 1., 1.};
      SUBCASE("To RGB") {
        Rgb rgb{hsv};
        CHECK_EQ(rgb, Rgb{1., 1., 0.});
      }
    }
  }
}

TEST_CASE("Color defaults") {
  CHECK_EQ(Rgb{}, RgbNone);
  CHECK_EQ(Rgb(), jwezel::RgbNone);
  CHECK_EQ(string(Rgb{}), "RgbNone");
  CHECK_EQ(Rgb(1.), jwezel::RgbRed); // the other components default to 0
  CHECK_NE(jwezel::RgbBlack, RgbNone); // black is a color
  CHECK_EQ(Rgb(0., 0., 0.), jwezel::RgbBlack);
  CHECK_EQ(CharAttributes{}.fg, RgbNone);
  CHECK_EQ(Char{}.attributes.bg, RgbNone);
}

TEST_CASE("Text from CharAttributes") {
  const Text text{"ab", CharAttributes{jwezel::RgbRed, RgbBlue, bold, AttributeMode::mix}};
  CHECK_EQ(text.data[0][1].attributes.fg, jwezel::RgbRed);
  CHECK_EQ(text.data[0][1].attributes.bg, RgbBlue);
  CHECK_EQ(text.data[0][1].attributes.attr, bold);
  CHECK_EQ(text.data[0][1].attributes.mix, AttributeMode::mix); // used to be dropped
  CHECK_EQ(Text{"ab", CharAttributes{}}.data[0][0].attributes.mix, AttributeMode::default_);
}

TEST_CASE("Attribute equality") {
  const CharAttributes a{jwezel::RgbRed, RgbBlue, bold, AttributeMode::merge};
  CHECK(a == CharAttributes{jwezel::RgbRed, RgbBlue, bold, AttributeMode::replace}); // the mix is not compared
  CHECK_FALSE(a != CharAttributes{jwezel::RgbRed, RgbBlue, bold});
  CHECK(Char('a', a) == Char('a', jwezel::RgbRed, RgbBlue, bold, AttributeMode::ignore));
  CHECK(a != CharAttributes{RgbGreen, RgbBlue, bold, AttributeMode::merge}); // one difference is enough
  CHECK(a != CharAttributes{jwezel::RgbRed, RgbGreen, bold, AttributeMode::merge});
  CHECK(a != CharAttributes{jwezel::RgbRed, RgbBlue, underline, AttributeMode::merge});
  CHECK_FALSE(a == CharAttributes{RgbGreen, RgbBlue, bold, AttributeMode::merge});
  CHECK(Char('a', a) != Char('b', a));
}

TEST_CASE("Color combining") {
  const auto transparent{jwezel::RgbTransparent};
  SUBCASE("Override") {
    CHECK_EQ(jwezel::RgbRed | RgbBlue, RgbBlue); // a specified color overrides
    CHECK_EQ(jwezel::RgbRed | RgbNone, jwezel::RgbRed); // unspecified or transparent ones do not
    CHECK_EQ(jwezel::RgbRed | transparent, jwezel::RgbRed);
    CHECK_EQ(RgbNone | RgbBlue, RgbBlue);
    CHECK_EQ(transparent | RgbBlue, RgbBlue);
    CHECK_EQ(RgbNone | transparent, RgbNone);
    CHECK_EQ(transparent | RgbNone, transparent);
  }
  SUBCASE("Mix") {
    CHECK_EQ(jwezel::RgbRed + RgbBlue, Rgb{0.5, 0., 0.5});
    CHECK_EQ(jwezel::RgbRed + RgbNone, jwezel::RgbRed);
    CHECK_EQ(RgbNone + RgbBlue, RgbBlue);
    CHECK_EQ(transparent + RgbBlue, RgbBlue);
    CHECK_EQ(jwezel::RgbRed + transparent, jwezel::RgbRed);
    CHECK_EQ(RgbNone + transparent, RgbNone);
  }
  SUBCASE("Char combine") {
    const Char destination{'a', jwezel::RgbRed, RgbBlue, bold};
    const Char source{'b', RgbNone, RgbGreen, underline};
    SUBCASE("merge: the source wins where it specifies, the destination shows through elsewhere") {
      const auto result{destination.combine(Char{'b', RgbNone, RgbGreen, underline, AttributeMode::merge})};
      CHECK_EQ(result.rune, 'b');
      CHECK_EQ(result.attributes.fg, jwezel::RgbRed);
      CHECK_EQ(result.attributes.bg, RgbGreen);
      CHECK_EQ(result.attributes.attr, bold | underline);
    }
    SUBCASE("mix blends") {
      const auto result{destination.combine(Char{'b', RgbNone, RgbGreen, underline, AttributeMode::mix})};
      CHECK_EQ(result.attributes.fg, jwezel::RgbRed);
      CHECK_EQ(result.attributes.bg, Rgb{0., 0.5, 0.5});
    }
    SUBCASE("replace") {
      CHECK_EQ(destination.combine(source, AttributeMode::replace).attributes.fg, RgbNone);
    }
    SUBCASE("A source rune replaces, whatever the mode") {
      for (auto mode: {AttributeMode::merge, AttributeMode::mix, AttributeMode::replace, AttributeMode::ignore}) {
        CHECK_EQ(destination.combine(Char{'b', RgbNone, RgbGreen, {}, mode}).rune, 'b');
      }
    }
    SUBCASE("No source rune keeps the destination rune, whatever the mode") {
      for (auto mode: {AttributeMode::merge, AttributeMode::mix, AttributeMode::replace, AttributeMode::ignore}) {
        CHECK_EQ(destination.combine(Char{jwezel::NoneRune, RgbNone, RgbGreen, {}, mode}).rune, 'a');
      }
    }
    SUBCASE("Tint without a rune") {
      const auto result{destination.combine(Char{jwezel::NoneRune, RgbNone, RgbGreen, {}, AttributeMode::merge})};
      CHECK_EQ(result.rune, 'a');
      CHECK_EQ(result.attributes.bg, RgbGreen);
    }
  }
  SUBCASE("Merging text onto a colored background") {
    Text window{Char{' ', RgbNone, RgbBlue}, Vector{4, 1}};
    window.patch(Text{"hi", jwezel::RgbRed, RgbNone}, Vector{0, 0}, AttributeMode::merge);
    CHECK_EQ(window.data[0][0].rune, 'h');
    CHECK_EQ(window.data[0][0].attributes.fg, jwezel::RgbRed); // the text's own color
    CHECK_EQ(window.data[0][0].attributes.bg, RgbBlue); // the background shines through
    CHECK_EQ(window.data[0][3].attributes.bg, RgbBlue);
  }
}

TEST_CASE("Char") {
  SUBCASE("Char only") {
    Char ch('x');
    CHECK_EQ(ch, Char('x', RgbNone, RgbNone, Attributes(), AttributeMode()));
    SUBCASE("string") {
      CHECK_EQ(string(ch), "'x'(fg=RgbNone, bg=RgbNone, attr=, mix=AttributeMode::default_)");
    }
  }
  SUBCASE("Char only") {
    Char ch('a', RgbGray1, RgbGray2, bold, AttributeMode::mix);
    CHECK_EQ(ch, Char('a', RgbGray1, RgbGray2, bold, AttributeMode::mix));
    SUBCASE("string") {
      CHECK_EQ(string(ch), "'a'(fg=Rgb(r=0.1, g=0.1, b=0.1), bg=Rgb(r=0.2, g=0.2, b=0.2), attr=bold, mix=mix)");
    }
  }
}

TEST_CASE("Text") {
  CharAttributes attr(Rgb(1.), Rgb(0., 0., 1.), bold);
  SUBCASE("Default") {
    Text text;
    CHECK_EQ(string(text), "");
  }
  SUBCASE("One line") {
    Text text("test");
    CHECK_EQ(string(text), "test\n");
  }
  SUBCASE("Box of Chars") {
    Text text(Char('.', CharAttributes(RgbBlue, RgbCyan1, bold)), Vector(5, 4));
    CHECK_EQ(text, Text(".....\n.....\n.....\n.....", RgbBlue, RgbCyan1, bold));
  }
  SUBCASE("Two lines, fg red, bg blue, bold") {
    Text text("line1\nline 2", attr);
    SUBCASE("String") {
      CHECK_EQ(string(text), "line1 \nline 2\n");
    }
    SUBCASE("height") {
      CHECK_EQ(text.height(), 2);
    }
    SUBCASE("width") {
      CHECK_EQ(text.width(), 6);
    }
    SUBCASE("size") {
      CHECK_EQ(text.size(), Vector{6, 2});
    }
    SUBCASE("Equal") {
      CHECK_EQ(text, Text("line1\nline 2", attr));
    }
    SUBCASE("Not equal") {
      CHECK(not (text == Text("line3\nline 2", attr)));
    }
    SUBCASE("Not equal (attribute)") {
      CHECK(not (text == Text("line1\nline 2", Rgb(1.), Rgb(0., 0., 1.), underline)));
    }
    SUBCASE("Right align (string comparison)") {
      CHECK_EQ(string(text.rightAligned(8)), "   line1\n  line 2\n");
    }
    SUBCASE("Right align") {
      CHECK_EQ(text.rightAligned(8), Text("   line1\n  line 2", attr));
    }
    SUBCASE("Right align (clip)") {
      CHECK_EQ(text.rightAligned(4), Text("ine1\nne 2", attr));
    }
    SUBCASE("Center ") {
      CHECK_EQ(text.centered(10), Text("  line1   \n  line 2", attr));
    }
    SUBCASE("Center (size fit)") {
      CHECK_EQ(text.centered(5), Text("line1\nline ", attr));
    }
    SUBCASE("Center (clip)") {
      CHECK_EQ(text.centered(3), Text("ine\nine", attr));
    }
    SUBCASE("Extend") {
      auto t2 = text;
      t2.extend(Vector(12, 4), Char('.', attr));
      CHECK_EQ(t2, Text("line1 ......\nline 2......\n............\n............", attr));
    }
    SUBCASE("Extend empty") {
      Text t2;
      t2.extend(Vector(12, 4), Char('.', attr));
      CHECK_EQ(t2, Text("............\n............\n............\n............", attr));
    }
    SUBCASE("Extend zero width") {
      Text t2{Text{"\n"}};
      CHECK_EQ(t2.width(), 0);
      t2.extend(Vector(4, 2), Char('.', attr));
      CHECK_EQ(t2, Text("....\n....", attr));
    }
    SUBCASE("Patch") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(1, 0));
      CHECK_EQ(t2, Text("lxxe1\nlyye 2", attr));
    }
    SUBCASE("Patch (off bottom)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(1, 1));
      CHECK_EQ(t2, Text("line1\nlxxe 2", attr));
    }
    SUBCASE("Patch (off right bottom)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(5, 1));
      CHECK_EQ(t2, Text("line1\nline x", attr));
    }
    SUBCASE("Patch (off left top)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(-1, -1));
      CHECK_EQ(t2, Text("yine1\nline 2", attr));
    }
    SUBCASE("Patch (way off bottom)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(1, 2));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("Patch (way off right bottom)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(6, 1));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("Patch (way off left top)") {
      auto t2 = text;
      t2.patch(Text("xx\nyy", attr), Vector(-2, -1));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("Patch (partly off left)") {
      auto t2 = text;
      t2.patch(Text("abcd", attr), Vector(-2, 0)); // what is outside is not copied, the rest is the right part
      CHECK_EQ(t2, Text("cdne1\nline 2", attr));
    }
    SUBCASE("Patch (partly off right)") {
      auto t2 = text;
      t2.patch(Text("abcd", attr), Vector(3, 0)); // the lines of the text are 6 wide
      CHECK_EQ(t2, Text("linabc\nline 2", attr));
    }
    SUBCASE("Patch (at or beyond the right edge)") {
      auto t2 = text;
      t2.patch(Text("ab", attr), Vector(6, 0)); // used to run off the end of the line
      CHECK_EQ(t2, text);
      t2.patch(Text("ab", attr), Vector(8, 1));
      CHECK_EQ(t2, text);
      t2.patch(Text("ab", attr), Vector(5, 1)); // the last cell is inside
      CHECK_EQ(t2, Text("line1\nline a", attr));
    }
    SUBCASE("setAttr") {
      auto t2 = text;
      t2.setAttr(CharAttributes(RgbGreen, RgbYellow, jwezel::reverse, jwezel::AttributeMode::replace), Rectangle{0, 0, 2, 2});
      CHECK_EQ(t2.data[0][0], Char('l', RgbGreen, RgbYellow, jwezel::reverse, AttributeMode::replace)); // runes stay
      CHECK_EQ(t2.data[1][1], Char('i', RgbGreen, RgbYellow, jwezel::reverse, AttributeMode::replace));
      CHECK_EQ(t2.data[0][2], text.data[0][2]); // outside of the area nothing changes
      CHECK_EQ(t2.data[1][2], text.data[1][2]);
      auto t3 = text;
      t3.setAttr(CharAttributes(RgbNone, RgbNone, jwezel::reverse), Rectangle{0, 0, 2, 1});
      CHECK_EQ(t3.data[0][0].attributes.fg, text.data[0][0].attributes.fg); // merged: only the attribute is added
      CHECK_EQ(t3.data[0][0].attributes.attr, text.data[0][0].attributes.attr | jwezel::reverse);
      CHECK_EQ(t3.data[0][2], text.data[0][2]);
    }
  }
  SUBCASE("Five lines, fg red, bg blue, bold") {
    Text text("line1\nline 2\nline 3\nline 4\nline 5", attr);
    SUBCASE("patchArea") {
      auto t2 = text;
      t2.patchArea(Text("12\n34", attr), Rectangle{1, 1, 3, 3});
      CHECK_EQ(t2, Text("line1\nl12e 2\nl34e 3\nline 4\nline 5", attr));
    }
    SUBCASE("patchArea not in text (low)") {
      auto t2 = text;
      CHECK_THROWS(t2.patchArea(Text("12\n34", attr), Rectangle{0, -1, 2, 1}));
    }
    SUBCASE("patchArea not in text (high)") {
      auto t2 = text;
      CHECK_THROWS(t2.patchArea(Text("12\n34", attr), Rectangle{0, 4, 2, 6}));
    }
    SUBCASE("operator []") {
      CHECK_EQ(text[Rectangle{3, 1, 6, 3}], Text("e 2\ne 3", attr));
    }
    SUBCASE("operator []") {
      CHECK_EQ(text[Vector{3, 1}], Char('e', attr));
    }
    SUBCASE("at") {
      CHECK_EQ(text.at(Vector{3, 1}), Char('e', attr));
    }
    SUBCASE("at out of bounds") {
      CHECK_THROWS(text.at(Vector{10, 1}));
    }
  }
  SUBCASE("Box drawing") {
    Text text(Char('.', CharAttributes(RgbBlue, RgbCyan1, bold)), Vector(5, 4));
    text.box(Box(Rectangle(0, 0, 5, 4), 2));
    CHECK_EQ(string(text), "┏━━━┓\n┃...┃\n┃...┃\n┗━━━┛\n");
    SUBCASE("Vertical line in box") {
      text.line(Line{Vector{0, 1}, DimHigh, jwezel::Horizontal, false, false});
      CHECK_EQ(string(text), "┏━━━┓\n┠───┨\n┃...┃\n┗━━━┛\n");
    }
  }
  SUBCASE("Box styles") {
    const auto drawn{
      [](u1 strength, u1 dash, bool rounded) {
        Text text(Char('.'), Vector(5, 4));
        text.box(Box(Rectangle(0, 0, 5, 4), strength, dash, rounded));
        return text;
      }
    };
    CHECK_EQ(string(drawn(1, 0, false)), "┌───┐\n│...│\n│...│\n└───┘\n");
    CHECK_EQ(string(drawn(2, 0, false)), "┏━━━┓\n┃...┃\n┃...┃\n┗━━━┛\n");
    CHECK_EQ(string(drawn(1, 2, false)), "┌╌╌╌┐\n╎...╎\n╎...╎\n└╌╌╌┘\n"); // used to have no vertical lines
    CHECK_EQ(string(drawn(1, 1, false)), "┌╌╌╌┐\n╎...╎\n╎...╎\n└╌╌╌┘\n"); // there is one dashed form
    CHECK_EQ(string(drawn(2, 2, false)), "┏╍╍╍┓\n╏...╏\n╏...╏\n┗╍╍╍┛\n");
    CHECK_EQ(string(drawn(1, 0, true)), "╭───╮\n│...│\n│...│\n╰───╯\n"); // used to damage the corners
    CHECK_EQ(string(drawn(1, 2, true)), "╭╌╌╌╮\n╎...╎\n╎...╎\n╰╌╌╌╯\n");
    CHECK_EQ(string(drawn(2, 0, true)), "┏━━━┓\n┃...┃\n┃...┃\n┗━━━┛\n"); // there are no heavy rounded corners
    for (u1 strength = 1; strength <= 2; ++strength) {
      for (u1 dash = 0; dash <= 2; ++dash) {
        for (const auto rounded: {false, true}) {
          for (const auto &line: drawn(strength, dash, rounded).data) {
            for (const auto &ch: line) {
              CHECK_NE(ch.rune, 0); // never a character that is not there
            }
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
