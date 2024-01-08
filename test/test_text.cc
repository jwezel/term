#include <term/geometry.hh>
#include <term/text.hh>

#include <doctest/doctest.h>

using
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
      CHECK(!(text == Text("line3\nline 2", attr)));
    }
    SUBCASE("Not equal (attribute)") {
      CHECK(!(text == Text("line1\nline 2", Rgb(1.), Rgb(0., 0., 1.), underline)));
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
    SUBCASE("setAttr") {
      auto t2 = text;
      t2.setAttr(CharAttributes(RgbGreen, RgbYellow, jwezel::reverse, jwezel::AttributeMode::replace), Rectangle{0, 0, 2, 2});
      CHECK_EQ(t2, Text("line1\nline 2", attr));
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
