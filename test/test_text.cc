#include <doctest/doctest.h>
#include "text.hh"
#include "geometry.hh"

using namespace jwezel;

namespace doctest {
    template<> struct StringMaker<Text> {
        static String convert(const Text& value) {
            return value.repr().c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<Rgb> {
        static String convert(const Rgb& value) {
            return string(value).c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<Hsv> {
        static String convert(const Hsv& value) {
            return string(value).c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<CharAttributes> {
        static String convert(const CharAttributes& value) {
            return string(value).c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<Char> {
        static String convert(const Char& value) {
            return string(value).c_str();
        }
    };
}

TEST_CASE("Color") {
  SUBCASE("Rgb") {
    Rgb c{1., 0., 0.};
    SUBCASE("Mix") {
      Rgb r{c + Rgb{0., 1., 0.}};
      CHECK_EQ(r, Rgb{0.5, 0.5, 0.});
    }
    SUBCASE("To HSV") {
      Hsv r = c;
      CHECK_EQ(r, Hsv{0., 1., 1.});
    }
  }
  SUBCASE("Hsv") {
    SUBCASE("Red") {
      Hsv c{0., 1., 1.};
      SUBCASE("To RGB") {
        Rgb r = c;
        CHECK_EQ(r, Rgb{1., 0., 0.});
      }
    }
    SUBCASE("Green") {
      Hsv c{120., 1., 1.};
      SUBCASE("To RGB") {
        Rgb r = c;
        CHECK_EQ(r, Rgb{0., 1., 0.});
      }
    }
    SUBCASE("Blue") {
      Hsv c{240., 1., 1.};
      SUBCASE("To RGB") {
        Rgb r = c;
        CHECK_EQ(r, Rgb{0., 0., 1.});
      }
    }
    SUBCASE("Yellow") {
      Hsv c{60., 1., 1.};
      SUBCASE("To RGB") {
        Rgb r = c;
        CHECK_EQ(r, Rgb{1., 1., 0.});
      }
    }
  }
}

TEST_CASE("Char") {
  SUBCASE("Char only") {
    Char c('x');
    CHECK_EQ(c, Char('x', RgbNone, RgbNone, Attributes(), AttributeMode()));
    SUBCASE("string") {
      CHECK_EQ(string(c), "'x'(fg=RgbNone, bg=RgbNone, attr=, mix=default_)");
    }
  }
  SUBCASE("Char only") {
    Char c('a', RgbGray1, RgbGray2, bold, mix);
    CHECK_EQ(c, Char('a', RgbGray1, RgbGray2, bold, mix));
    SUBCASE("string") {
      CHECK_EQ(string(c), "'a'(fg=Rgb(r=0.1, g=0.1, b=0.1), bg=Rgb(r=0.2, g=0.2, b=0.2), attr=bold, mix=mix)");
    }
  }
}

TEST_CASE("Text") {
  CharAttributes attr(Rgb(1.), Rgb(0., 0., 1.), bold);
  SUBCASE("Default") {
    Text t;
    CHECK_EQ(string(t), "");
  }
  SUBCASE("One line") {
    Text t("test");
    CHECK_EQ(string(t), "test\n");
  }
  SUBCASE("Box of Chars") {
    Text t(Char('.', CharAttributes(RgbBlue, RgbCyan1, bold)), Vector(5, 4));
    CHECK_EQ(t, Text(".....\n.....\n.....\n.....", RgbBlue, RgbCyan1, bold, jwezel::merge));
  }
  SUBCASE("Two lines, fg red, bg blue, bold") {
    Text t("line1\nline 2", attr);
    SUBCASE("String") {
      CHECK_EQ(string(t), "line1 \nline 2\n");
    }
    SUBCASE("height") {
      CHECK_EQ(t.height(), 2);
    }
    SUBCASE("width") {
      CHECK_EQ(t.width(), 6);
    }
    SUBCASE("size") {
      CHECK_EQ(t.size(), Vector{6, 2});
    }
    SUBCASE("Equal") {
      CHECK_EQ(t, Text("line1\nline 2", attr));
    }
    SUBCASE("Not equal") {
      CHECK(!(t == Text("line3\nline 2", attr)));
    }
    SUBCASE("Not equal (attribute)") {
      CHECK(!(t == Text("line1\nline 2", Rgb(1.), Rgb(0., 0., 1.), underline)));
    }
    SUBCASE("Right align (string comparison)") {
      CHECK_EQ(string(t.rightAligned(8)), "   line1\n  line 2\n");
    }
    SUBCASE("Right align") {
      CHECK_EQ(t.rightAligned(8), Text("   line1\n  line 2", attr));
    }
    SUBCASE("Right align (clip)") {
      CHECK_EQ(t.rightAligned(4), Text("ine1\nne 2", attr));
    }
    SUBCASE("Center ") {
      CHECK_EQ(t.centered(10), Text("  line1   \n  line 2", attr));
    }
    SUBCASE("Center (size fit)") {
      CHECK_EQ(t.centered(5), Text("line1\nline ", attr));
    }
    SUBCASE("Center (clip)") {
      CHECK_EQ(t.centered(3), Text("ine\nine", attr));
    }
    SUBCASE("Extend") {
      auto t2 = t;
      t2.extend(Vector(12, 4), Char('.', attr));
      CHECK_EQ(t2, Text("line1 ......\nline 2......\n............\n............", attr));
    }
    SUBCASE("Patch") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(1, 0));
      CHECK_EQ(t2, Text("lxxe1\nlyye 2", attr));
    }
    SUBCASE("Patch (off bottom)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(1, 1));
      CHECK_EQ(t2, Text("line1\nlxxe 2", attr));
    }
    SUBCASE("Patch (off right bottom)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(5, 1));
      CHECK_EQ(t2, Text("line1\nline x", attr));
    }
    SUBCASE("Patch (off left top)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(-1, -1));
      CHECK_EQ(t2, Text("yine1\nline 2", attr));
    }
    SUBCASE("Patch (way off bottom)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(1, 2));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("Patch (way off right bottom)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(6, 1));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("Patch (way off left top)") {
      auto t2 = t;
      t2.patch(Text("xx\nyy", attr), Vector(-2, -1));
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
    SUBCASE("setAttr") {
      auto t2 = t;
      t2.setAttr(CharAttributes(RgbGreen, RgbYellow, jwezel::reverse, jwezel::replace), Rectangle{0, 0, 2, 2});
      CHECK_EQ(t2, Text("line1\nline 2", attr));
    }
  }
  SUBCASE("Five lines, fg red, bg blue, bold") {
    Text t("line1\nline 2\nline 3\nline 4\nline 5", attr);
    SUBCASE("patchArea") {
      auto t2 = t;
      t2.patchArea(Text("12\n34", attr), Rectangle{1, 1, 3, 3});
      CHECK_EQ(t2, Text("line1\nl12e 2\nl34e 3\nline 4\nline 5", attr));
    }
    SUBCASE("patchArea not in text (low)") {
      auto t2 = t;
      CHECK_THROWS(t2.patchArea(Text("12\n34", attr), Rectangle{0, -1, 2, 1}));
    }
    SUBCASE("patchArea not in text (high)") {
      auto t2 = t;
      CHECK_THROWS(t2.patchArea(Text("12\n34", attr), Rectangle{0, 4, 2, 6}));
    }
    SUBCASE("operator []") {
      CHECK_EQ(t[Rectangle{3, 1, 6, 3}], Text("e 2\ne 3", attr));
    }
    SUBCASE("operator []") {
      CHECK_EQ(t[Vector{3, 1}], Char('e', attr));
    }
    SUBCASE("at") {
      CHECK_EQ(t.at(Vector{3, 1}), Char('e', attr));
    }
    SUBCASE("at out of bounds") {
      CHECK_THROWS(t.at(Vector{10, 1}));
    }
  }
  SUBCASE("Box drawing") {
    Text t(Char('.', CharAttributes(RgbBlue, RgbCyan1, bold)), Vector(5, 4));
    t.box(Box(Rectangle(0, 0, 5, 4), 2));
    CHECK_EQ(string(t), "┏━━━┓\n┃...┃\n┃...┃\n┗━━━┛\n");
    SUBCASE("Vertical line in box") {
      t.line(Line{Vector{0, 1}, DimHigh, jwezel::Horizontal, false, false});
      CHECK_EQ(string(t), "┏━━━┓\n┠───┨\n┃...┃\n┗━━━┛\n");
    }
  }
}
